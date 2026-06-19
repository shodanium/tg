// g++ -O2 -I. hnsw.cpp
// cl /O2 /EHsc /I. hnsw.cpp
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>

// oppa one-line style!
using std::vector;
using Hnsw = vector<vector<vector<int>>>;
struct Match { float dist; int key; };

bool operator<(const Match & a, const Match & b) { return a.dist < b.dist; }
bool operator>(const Match & a, const Match & b) { return b < a; }

// hardcoded vector count, dimensions, and query vector index
const int N = 1000;
const int DIM = 100;
const int QUERY = 123; // [0,N-1)
const char * VECS_FILE = "glove10k.bin";
const char * INDEX_FILE = "glove10k.hnsw";

// our distance function
float l2sq(const vector<float> & a, const vector<float> & b)
{
	float d = 0;
	for (int i = 0; i < a.size(); i++)
		d += (a[i] - b[i]) * (a[i] - b[i]);
	return d;
}

// fun stuff: our main search function!
vector<Match> SearchHnsw(const Hnsw & nodes, const vector<vector<float>> & vecs,
	const vector<float> & query, int entry, int max_level, int k)
{
	// greedy descent from max_level down to 1
	for (int level = max_level; level >= 1; level--)
	{
		float best = l2sq(query, vecs[entry]);
		bool changed;
		do
		{
			changed = false;
			for (int nb : nodes[entry][level]) // nb for neighbor
			{
				float d = l2sq(query, vecs[nb]);
				if (d < best)
				{
					best = d;
					entry = nb;
					changed = true;
				}
			}
		} while (changed);
	}

#if 0
	// or alternatively... *random* entry point for level 0 also works (that's NSW w/o H)
	entry = (rand() * RAND_MAX + rand()) % nodes.size();
#endif

	// beam search at level 0 with ef = max(default_ef, k)
	std::priority_queue<Match, vector<Match>, std::greater<Match>> cands; // min-heap: closest first
	std::priority_queue<Match> results; // max-heap: farthest first
	std::unordered_set<int> visited;

	auto Visit = [&](int slot) {
		if (!visited.insert(slot).second)
			return;
		float d = l2sq(query, vecs[slot]);
		cands.push({ d, slot });
		results.push({ d, slot });
		if (results.size() > std::max(64, k))
			results.pop();
	};

	Visit(entry);
	while (!cands.empty())
	{
		Match top = cands.top();
		if (results.size() >= k && top.dist > results.top().dist)
			break; // all remaining are farther than our ef-th result
		cands.pop();
		for (int nb : nodes[top.key][0])
			Visit(nb);
	}

	// build final result (trim to k, and sort)
	vector<Match> out;
	while (!results.empty())
	{
		if (results.size() <= k)
			out.push_back(results.top());
		results.pop();
	}
	std::reverse(out.begin(), out.end());
	return out;
}

// boring helpers
void pdie(const char * why) { perror(why); exit(1); }
void xfread(void * buf, int n, FILE * fp, const char * why) { if (fread(buf, 1, n, fp) != n) pdie(why); }
FILE * xfopen(const char * name) { FILE * fp = fopen(name, "rb"); if (!fp) pdie("fopen"); return fp; }
int64_t xreadint64(FILE * fp, int n) { int64_t v = 0; xfread(&v, n, fp, "int"); return v; }
int xreadint(FILE * fp, int n) { return (int)xreadint64(fp, n); }

void LoadHnsw(Hnsw & nodes, int & entry_slot, int & max_level, vector<int64_t> & keys)
{
	FILE * fp = xfopen(INDEX_FILE);
	fseek(fp, 64, SEEK_CUR); // skip "index" header
	int size = xreadint(fp, 8); // expect N
	int conn = xreadint(fp, 8); // default is 16
	int conn_base = xreadint(fp, 8); // default is 32
	max_level = xreadint(fp, 8);
	entry_slot = xreadint(fp, 8);
	fseek(fp, 2 * size, SEEK_CUR); // skip levels map

	vector<int> buf(conn_base); // expect conn_base >= conn
	nodes.resize(size);
	for (auto & node : nodes)
	{
		keys.push_back(xreadint64(fp, 8)); // load keys
		node.resize(xreadint(fp, 2) + 1); // read the max level, resize the node
		for (int l = 0; l < node.size(); l++)
		{
			int count = (int)xreadint(fp, 4); // how many neighbors are actually used
			xfread(buf.data(), 4 * (l ? conn : conn_base), fp, "neighbors");
			node[l].assign(buf.begin(), buf.begin() + count);
		}
	}
	fclose(fp);
}

#include <usearch/index.hpp>
#include <usearch/index_dense.hpp>
using namespace unum::usearch;

void MakeUsearchIndex(vector<vector<float>> & vecs, int dim, int n, const vector<float> & query)
{
	// create and build the USearch index
	metric_punned_t metric(dim, metric_kind_t::l2sq_k, scalar_kind_t::f32_k);
	index_dense_t index = index_dense_t::make(metric);
	index.reserve(n);
	for (int i = 0; i < n; i++)
		index.add(1'000'000LL * std::rand() + i, vecs[i].data());

	// search the USearch index for reference
	auto r = index.search(query.data(), 5);
	printf("USearch results\n");
	for (int i = 0; i < r.size(); i++)
		printf("%d. key %lld dist %f\n", i + 1, (long long int)r[i].member.key, r[i].distance);

	// and save it (true == exclude the vectors)
	index.save(INDEX_FILE, { true, false });
}

int main(int argc, char ** argv)
{
	// load our vectors!
	vector<vector<float>> vecs(N);

	FILE * fp = xfopen(VECS_FILE);
	for (auto & v : vecs)
	{
		v.resize(DIM);
		xfread(v.data(), 4 * DIM, fp, "vecs");
	}
	fclose(fp);

	// build, search, and save the USearch index
	MakeUsearchIndex(vecs, DIM, N, vecs[QUERY]);

	// now load its data manually
	Hnsw nodes;
	int entry_slot, max_level;
	vector<int64_t> keys;
	LoadHnsw(nodes, entry_slot, max_level, keys);

#if 0
	// note: `LoadHnsw()` returns `entry_slot` and `max_level` for us, but
	// we can *recompute* them from `nodes` just as well
	//
	// and so `nodes` *does* suffice for searching, as promised!
	int level;
	entry_slot = max_level = -1;
	for (int i = 0; i < nodes.size(); i++)
		if (max_level < (level = (int)nodes[i].size() - 1))
		{
			max_level = level;
			entry_slot = i;
		}
#endif

	// and search it manually
	auto r = SearchHnsw(nodes, vecs, vecs[QUERY], entry_slot, max_level, 5);
	printf("manual HNSW results\n");
	for (int i = 0; i < r.size(); i++)
		printf("%d. key %lld dist %f\n", i + 1, (long long int)keys[r[i].key], r[i].dist);
	return 0;
}
