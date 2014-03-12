#include "astar.h"
#include "astar2way.h"

typedef struct {
  int id;
  SIGNATURE(id*1);  // for duplicate detection as well as for debug output
} tNode;  // a node in our graph is simply an ID

typedef struct {
  int src_id;
  int dest_id;
  SIGNATURE(src_id*1 / dest_id*2);  // for debug output only
} tEdge;  // an edge in our graph is a pair of node IDs

const int gNumNodes = 10;

class SimpleGraphSearch : public AStar<tNode, tEdge, float> {
 public:
  SimpleGraphSearch() {
    // initialize the graph
    for (int i = 0; i < gNumNodes; i++)
      for (int j = 0; j < gNumNodes; j++)
        edge_[i][j] = -1;
    // add a few links
    AddEdge(0, 1, 10);
    AddEdge(1, 2, 20);
    AddEdge(0, 4, 1);
    AddEdge(4, 2, 2);
    AddEdge(2, 3, 10);
    AddEdge(3, 5, 20);
    AddEdge(2, 5, 25);
    AddEdge(0, 5, 32);
    AddEdge(3, 6, 100);
  }
  ~SimpleGraphSearch() {};

 protected:
  // Required virtual functions to override

  // virtual function 1:
  //   ExpandNode:  expand a node and lists all candidates.
  //                (as a vector of tExpandCandidates records)
  virtual int ExpandNode(const tNode& src,
                         const tNode *prev_node,
                         const tEdge *prev_edge,
                         float cumulative_cost,
                         vector<tExpandCandidate> *result) {
    for (int i = 0; i < gNumNodes; i++)
      if (edge_[src.id][i] >= 0) {
        tExpandCandidate c;
        c.edge.src_id = src.id;
        c.edge.dest_id = i;
        c.edge_cost = edge_[src.id][i];
        c.newnode.id = i;
        result->push_back(c);
      }
    return result->size();
  }

  // virtual function 2:  (one-way search only)
  //   IsTarget:  checks whether a node is the target (success) node
  virtual bool IsTarget(const tNode& node, float cost_so_far) {
    return (node.id == 5);
  }

  // virtual function 3:
  //   EstimateRemainingCost: calculates remaining cost heuristics (h function)
  //                        (note: must be admissible -- always underestimate!)
  virtual float EstimateRemainingCost(const tNode& node) {
    return 0;
  }

 private:
  int edge_[gNumNodes][gNumNodes];

  void AddEdge(int p1, int p2, int weight) {
    ASSERT(p1 >= 0 && p1 < gNumNodes);
    ASSERT(p2 >= 0 && p2 < gNumNodes);
    edge_[p1][p2] = weight;
    edge_[p2][p1] = weight;
  }
};



// one-way A* search test
void TestOneWaySearch() {
  SimpleGraphSearch search;

  // source node
  tNode src;
  src.id = 0;
  search.AddSrcNode(src);

  // start searching
  vector<SimpleGraphSearch::tPlanStep> result;
  float cost = search.Search(&result);

  LOG(INFO) << "open list: " << search.NumOpenNodes()
         << ", closed list: " << search.NumClosedNodes();

  LOG(INFO) << "Best plan: cost = " << cost;
  for (int i = 0; i < result.size(); i++) {
    if (i > 0) {
      LOG(INFO) << "Edge: (" << result[i].edge->src_id << ", "
             << result[i].edge->dest_id << ") cost = " << result[i].edge_cost;
    }
    LOG(INFO) << "Node: " << result[i].node->id;
  }

  ASSERT(cost == 28);
}


// two-way A* search test
void TestTwoWaySearch() {
  AStarTwoWay<SimpleGraphSearch> search_2way;

  // source node
  tNode src;
  src.id = 0;
  search_2way.AddSrcNode(src);

  // destination node
  tNode dest;
  dest.id = 5;
  search_2way.AddDestNode(dest);

  // start searching
  vector<SimpleGraphSearch::tPlanStep> result;
  float cost = search_2way.Search(&result);

  LOG(INFO) << "forward direction -- open list: "
         << search_2way.NumForwardOpenNodes()
         << ", closed list: " << search_2way.NumForwardClosedNodes();
  LOG(INFO) << "reverse direction -- open list: "
         << search_2way.NumReverseOpenNodes()
         << ", closed list: " << search_2way.NumReverseClosedNodes();

  LOG(INFO) << "Best plan: cost = " << cost;
  for (int i = 0; i < result.size(); i++) {
    if (i > 0) {
      LOG(INFO) << "Edge: (" << result[i].edge->src_id << ", "
             << result[i].edge->dest_id << ") cost = " << result[i].edge_cost;
    }
    LOG(INFO) << "Node: " << result[i].node->id;
  }

  ASSERT(cost == 28);
}

int main() {

  LOG(INFO) << "*** Testing one-way A* search...";
  TestOneWaySearch();

  LOG(INFO) << "*** Testing two-way A* search...";
  TestTwoWaySearch();

  LOG(INFO) << "PASS";

  return 0;
}
