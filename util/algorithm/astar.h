#ifndef _PUBLIC_UTIL_ALGORITHM_ASTAR_H_
#define _PUBLIC_UTIL_ALGORITHM_ASTAR_H_

#include "nodelist.h"
#include "util/memory/collection.h"

// A* search algorithm, modified for OpTrip travel planning (OA*)
//
// -- modified from the traditional A* algorithm to allow node merging
// and weak-duplicate vs. strong-duplicate concepts.
//
// The concept of weak-duplicate vs. strong-duplicate is user-defined.
// In terms of our A* implementation, strong duplicates are always pruned --
// whenever two nodes are detected as strong duplicates, the one with
// larger g-value (cost) is eliminated.
// On the other hand, weak duplicates are allowed, and lead to node merging:
// when two nodes are detected as weak duplicates, the two branches are
// merged into the same node, while both parent links are remembered.
// Merged nodes must have identical h-value (remaining cost estimate).
//
// This is useful to speed up complex travel planning: weak-duplicate can
// be defined as "time + location + remaining tasks", so that two flights
// arriving at roughly the same time can be merged into one branch to
// speed up subsequent searching.  Two nodes are strong duplicates only
// if they derive from the same travel plan.
// Note: In case parent links are examined when testing for strong duplicates,
//       please only consider the primary branch.  Strong duplicates with
//       non-primary branches should be treated as weak duplicates (leading
//       to some waste, but the algorithm is still correct), since we do not
//       support per-branch duplicate scenario.
//
//
// Weak duplicates are defined by SIGNATURE macro in tNode type.
// Strong duplicates are defined by StrongDuplicate() virtual function.
//
// By default, all weak duplicates are strong duplicates (StrongDuplicate()
// always returns true) -- this is the traditional A* algorithm, and only
// one parent exists for each non-root node.
//

// tNode is the type of a node, while tEdge is the type of an edge.
// For example, in case of driving directions search, tNode would contain
// longitude/latitude/z-value (XYZ) info of a point, and tEdge would contain
// reference to a road object.
//
// In A-star implementation below, AStar defines its own "node" (tAStarNode),
// which contains the user-defined tNode, as well as additional bookkeeping
// information.  Similarly, it defines its own "edge" (tAStarEdge), which
// contains user-defined tEdge, as well as additional information.
//
// tNode must include a SIGNATURE macro which spells out all of its fields.
// (They are used for weak-duplicate detection.)  Debug functions also depend
// on this SIGNATURE macro to generate debug output.
//
// tEdge should include a SIGNATURE macro for debug purposes.  So it is only
// necessary to spell out fields in tEdge that you want to appear in debug
// messages.

template<class tNode, class tEdge, class tCost = double>
class AStar {
 public:
  AStar() : searching_(false), is_oneway_search_(true),
            is_reverse_search_(false), success_node_(NULL) {
  };
  virtual ~AStar() {};

  bool searching() const  { return searching_; }

  void SetOneWaySearch() {
    is_oneway_search_ = true;
    is_reverse_search_ = false;
  }
  void SetForwardSearch() {
    // configure this search as forward search in a two-way A* algorithm
    // (IsTarget() is ignored)
    is_oneway_search_ = false;
    is_reverse_search_ = false;
  }
  void SetReverseSearch() {
    // configure this search as reverse search in a two-way A* algorithm
    // (IsTarget() is ignored)
    is_oneway_search_ = false;
    is_reverse_search_ = true;
  }

  // struct definitions: tAStarNode and tAStarEdge
  // (They're really simple structs, but defined as classes because otherwise
  //  the compiler would give errors at recursive reference claiming
  //  template type mismatch.  We still follow the naming convention for
  //   simple structs here, i.e., no trailing _ for field names.)
  class tAStarNode;
  class tAStarEdge {
   public:
    const tAStarNode *parent;
    tEdge edge;
    tCost edge_cost;
    tCost cumulative_cost;
  };
  class tAStarNode {
   public:
    tAStarNode() : nodelist_index(-1), twoway_connector(false) {};

    tNode node;
    vector<tAStarEdge> parent_link;
    tCost f_value;

    int nodelist_index:31;  // this field is accessed by NodeList only:
                            // if >=0, index in open list;
                            // if <0, index in closed list
    bool twoway_connector:1;  // special flag to indicate whether this node
                              // connects two nodes in two-way A* search
                              // to form a successful path
                              // (if true, "node" field must be target node,
                              //  and the corresponding "edge" is undefined.)

    tCost CumulativeCost() const {
      return (parent_link.empty() ? 0 : parent_link[0].cumulative_cost);
    }

    // signature of this structure is defined by the signature of tNode
    // (for weak-duplicate detection)
    SIGNATURE(node*1);
  };
  typedef tNode BaseNode;
  typedef tEdge BaseEdge;
  typedef tCost BaseCost;

  // add a source node src_ vector, and to open list
  inline void AddSrcNode(const tNode& src) {
    src_.push_back(src);
    AddInitialNode(src);
  }
  // add a destination node to dest_ vector (optional; may be helpful for
  // EstimateRemainingCost())
  inline void AddDestNode(const tNode& dest) {
    dest_.push_back(dest);
  }

  //
  // perform actual search
  //   If no plan has been found, return value will be -1, and result will be
  //      set to empty
  //   If a plan has been found, return value will be the lowest cost, and
  //      result will be a vector of tPlanStep structs, spelling out the full
  //      path (the first node will be the starting node and the first edge
  //      will be NULL)
  //
  typedef struct {
    const tEdge *edge;
    tCost earlier_cost;  // cumulative cost up until just before this step
    tCost edge_cost;     // cost of this step
    const tNode *node;
    inline tCost CumulativeCost() const { return earlier_cost + edge_cost; }
  } tPlanStep;
  tCost Search(vector<tPlanStep> *result);
  void SearchOneStep();

  inline int NumOpenNodes() const { return open_.size(); }
  inline int NumClosedNodes() const { return closed_.size(); }

  // debug output: give open/closed node count information
  string PrintNodeCount() const {
    stringstream ss;
    ss << "open " << NumOpenNodes() << " / closed " << NumClosedNodes();
    return ss.str();
  }

  const tAStarNode *FirstOpenNode() const {
    return open_.FirstNode();
  }

  // take the top node off of open list, and put it into closed list
  void MoveFirstOpenNodeToClosedList() {
    tAStarNode *current = open_.GetAndRemoveFirstNode();
    ASSERT(current != NULL);
    closed_.push_back(current);
  }

  // check for duplicates and add a new node to open list
  void RegisterNode(const tAStarNode& newnode);

  inline const tAStarNode *FindInClosedList(const tAStarNode *node) const {
    const tAStarNode *dup = nodestorage_.Lookup(*node);
    if (!dup)
      return NULL;  // not found in either open or closed list
    else if (open_.InList(dup))
      return NULL;  // found in open list -- ignore it
    else
      return dup;  // the node is found in closed list
  }

  // trace a plan from the given node
  void TracePlanFromNode(const tAStarNode *final_node,
                         vector<tPlanStep> *result) const;

  // for debugging: dump all steps of a plan
  static void DumpPlan(const vector<tPlanStep>& plan) {
    for (int i = 0; i < plan.size(); i++) {
      const tPlanStep& step = plan[i];
      VLOG(2) << (step.node == NULL ?
                 "---" :
                 serial::Serializer::ToJSON(*(step.node)))
             << (step.edge == NULL ? " (no edge)" : " via edge ")
             << (step.edge == NULL ? "" : serial::Serializer::ToJSON(*(step.edge)))
             << " (cost=" << plan[i].edge_cost << ") cumulative cost: "
             << plan[i].CumulativeCost();
    }
  }

 private:
  // add a source node to open list
  void AddInitialNode(const tNode& initial);

 protected:
  // trace a plan from the node being expanded
  void TracePlanFromExpansionCandidate(vector<tPlanStep> *result) const {
    ASSERT(!closed_.empty());
    TracePlanFromNode(closed_.back(), result);
  }

  //
  // Below are virtual functions that need to be overridden for each algorithm
  // that uses AStar framework.
  //

  // virtual function 1:
  //   ExpandNode:  expand a node and lists all candidates.
  //                (as a vector of tExpandCandidates records)
  typedef struct {
    tEdge edge;
    tCost edge_cost;
    tNode newnode;
  } tExpandCandidate;
  virtual int ExpandNode(const tNode& src,
                         const tNode *prev_node,
                         const tEdge *prev_edge,
                         tCost cumulative_cost,
                         vector<tExpandCandidate> *result) = 0;

  // virtual function 2:  (one-way A* search only)
  //   IsTarget:  checks whether a node is the target (success) node
  virtual bool IsTarget(const tNode& node, tCost cost_so_far) {
    return false;
  }

  // virtual function 3:
  //   EstimateRemainingCost: calculates remaining cost heuristics (h function)
  //                        (note: must be admissible -- always underestimate!)
  virtual tCost EstimateRemainingCost(const tNode& node) = 0;

  // virtual function 4:
  //   StrongDuplicate: detects strong duplicates
  //     (input: two nodes that are already determined to be weak-duplicates)
  virtual bool StrongDuplicate(const tNode& n1, const tNode& n2) const {
    return true;  // default: all weak duplicates are strong duplicates
  }

  //
  // used in two-way A* search only: expand node and estimate cost in reverse
  //
  //   (default: call their forward-search counterpart directly)
  virtual int ExpandNode_Reverse(const tNode& src,
                                 const tNode *prev_node,
                                 const tEdge *prev_edge,
                                 tCost cumulative_cost,
                                 vector<tExpandCandidate> *result) {
    return ExpandNode(src, prev_node, prev_edge, cumulative_cost, result);
  };

  // estimate remaining cost in reverse search (from current node to src node)
  //   (note: must be admissible -- always underestimate!)
  virtual tCost EstimateRemainingCost_Reverse(const tNode& node) {
    return EstimateRemainingCost(node);
  };

  //
  // end of two-way A* virtual function definitions
  //

  //
  // data variables
  //
  typedef UniqueCollection<tAStarNode> tNodeStorage;
  tNodeStorage nodestorage_;

  // src and dest nodes
  vector<tNode> src_, dest_;

  NodeList<tAStarNode> open_;    // open list -- a heap-based structure
  vector<tAStarNode *> closed_;  // closed list -- a simple vector
  bool searching_, is_oneway_search_, is_reverse_search_;
  const tAStarNode *success_node_;  // NULL if not yet successful
  tCost best_cost_;  // -1 if not successful
};


// implementations

// add an initial node to open list
template<class tNode, class tEdge, class tCost>
void AStar<tNode, tEdge, tCost>::AddInitialNode(const tNode& initial) {
  ASSERT(closed_.empty())
    << ("Each A-star object can only be used once.  "
        "Be sure to destroy the object and re-construct a new one "
        "before starting another search.");

  tAStarNode newnode;
  newnode.node = initial;
  newnode.parent_link.clear();
  newnode.f_value = 0;

  RegisterNode(newnode);

  searching_ = true;
}

// check for duplicates and add a new node to open list
template<class tNode, class tEdge, class tCost>
void AStar<tNode, tEdge, tCost>::RegisterNode(const tAStarNode& newnode) {
  // check if newnode has a weak duplicate in nodestorage_

  ASSERT(newnode.parent_link.size() <= 1);
  if (!(newnode.parent_link.empty())) {
    VLOG(3) << "-- via "
           << (newnode.twoway_connector ?
               "twoway connector to the other search direction" :
               serial::Serializer::ToJSON(newnode.parent_link[0].edge))
           << ", cost=" << newnode.parent_link[0].edge_cost;
  }

  VLOG(3) << "--- to node: " << serial::Serializer::ToJSON(newnode.node)
         << ", g=" << newnode.CumulativeCost() << ", f=" << newnode.f_value;

  tAStarNode *dup = nodestorage_.Lookup_Mutable(newnode);
  if (dup == NULL) {
    // no duplicate found

    // register the node in nodestorage_, and add it to open list
    tAStarNode *new_entry = nodestorage_.Insert_Mutable(newnode);
    open_.AddNode(new_entry);
    //    VLOG(3) << "Not a duplicate.";
  }
  else {
    // weak duplicate node exists

    VLOG(3) << "    Found a duplicate: "
           << "g=" << dup->CumulativeCost() << ", f=" << dup->f_value
           << ", parent = "
           << (dup->parent_link.size() > 0 ?
               serial::Serializer::ToJSON(dup->parent_link[0].parent->node)
               : "NULL");

    // check if it is a strong duplicate
    if (StrongDuplicate(newnode.node, dup->node)) {
      // eliminiate the node with higher g value
      ASSERT(newnode.parent_link.size() == 1);

      VLOG(3) << "      *** New node is a strong duplicate";

      if (newnode.CumulativeCost() < dup->CumulativeCost()) {
        // new node has lower cost -- replace dup with new node

        // sanity check: if we've found a strong duplicate with better
        //   cumulative cost, the dup cannot be root node
        ASSERT(dup->parent_link.size() > 0);

        VLOG(3) << "      *** with better cost.  Replacing...";

        //        dup->node = newnode.node;
        dup->parent_link[0] = newnode.parent_link[0];
        dup->f_value = newnode.f_value;
        dup->twoway_connector = newnode.twoway_connector;
        // f_value has changed; need to update its position in open list
        // (dup must be in open list -- fails if not)
        ASSERT(open_.AdjustPosition(dup))
          << "duplicate is not in open list.  "
          << "Please make sure your heuristics function is admissible!";
      }
      else {
        // new node has higher cost -- do nothing and ignore this new node
        VLOG(3) << "      *** and not any better.  Ignored.";
      }
    }
    else {
      // new node is a weak duplicate, but not a strong duplicate
      // let's keep both nodes (add newnode's parent link to dup's)

      VLOG(3) << "      *** New node is a weak duplicate";

      ASSERT(!(newnode.twoway_connector))
        << "Initial/target nodes cannot have weak duplicates!";

      if (newnode.CumulativeCost() < dup->CumulativeCost()) {
        // new node has lower cost than dup node's primary branch
        // -- replace dup's primary branch with new node

        // sanity check: if we've found a strong duplicate with better
        //   cumulative cost, the dup cannot be root node
        ASSERT(dup->parent_link.size() > 0);

        VLOG(3) << "      *** with better cost.  Redirecting primary branch...";

        dup->parent_link.push_back(dup->parent_link[0]);
        dup->parent_link[0] = newnode.parent_link[0];
        dup->f_value = newnode.f_value;
        // f_value has changed; need to update its position in open list
        // (dup must be in open list -- fails if not)
        ASSERT(open_.AdjustPosition(dup))
          << "duplicate is not in open list.  "
          << "Please make sure your heuristics function is admissible!";
      }
      else {
        // new node has higher cost than dup node's primary branch
        // -- keep new node's branch as a non-primary branch of dup's
        VLOG(3) << "      *** without better cost.  Keeping as alt branch...";
        dup->parent_link.push_back(newnode.parent_link[0]);
      }
    }
  }
}

// perform actual search
template<class tNode, class tEdge, class tCost>
tCost AStar<tNode, tEdge, tCost>::Search(vector<tPlanStep> *result) {
  searching_ = true;
  success_node_ = NULL;
  best_cost_ = -1;

  ASSERT(closed_.empty())
    << ("Each A-star object can only be used once.  "
        "Be sure to destroy the object and re-construct a new one "
        "before starting another search.");

  ASSERT(result != NULL);

  while (searching_)
    SearchOneStep();

  if (success_node_ != NULL) {
    TracePlanFromNode(success_node_, result);

    best_cost_ = success_node_->CumulativeCost();
  }
  else {
    result->clear();
    best_cost_ = -1;
  }
  return best_cost_;
}


// perform actual search for one step (called by Search())
template<class tNode, class tEdge, class tCost>
void AStar<tNode, tEdge, tCost>::SearchOneStep() {
  if (!searching_) return;

  const tAStarNode *current = open_.FirstNode();

  if (!current) {
    // Open list is empty.  The search has failed.
    searching_ = false;
    success_node_ = NULL;
  }
  else {
    ASSERT(current->twoway_connector == false);

    // put the current node into closed list
    VLOG(3) << (is_oneway_search_ ? "" :
               (is_reverse_search_ ? "reverse " : "forward "))
           << "[#" << closed_.size() << "] Expanding "
           << serial::Serializer::ToJSON(current->node)
           << ", g=" << current->CumulativeCost()
           << ", f=" << current->f_value;
    MoveFirstOpenNodeToClosedList();

    // check if the target has been reached
    if (is_oneway_search_ &&
        IsTarget(current->node, current->CumulativeCost())) {
      // target found
      VLOG(3) << "Target found!";
      searching_ = false;
      success_node_ = current;
    }
    else {
      // expand the current node
      vector<tExpandCandidate> candidates;

      const tNode *prev_node;
      const tEdge *prev_edge;
      if (current->parent_link.empty() ||
          current->parent_link[0].parent == NULL) {
        prev_node = NULL;
        prev_edge = NULL;
      }
      else {
        prev_node = &(current->parent_link[0].parent->node);
        prev_edge = &(current->parent_link[0].edge);
      }

      if (is_reverse_search_)
        ExpandNode_Reverse(current->node, prev_node, prev_edge,
                           current->CumulativeCost(),
                           &candidates);
      else
        ExpandNode(current->node, prev_node, prev_edge,
                   current->CumulativeCost(),
                   &candidates);
      int num_new_nodes = candidates.size();

      // VLOG(3) << num_new_nodes << " candidates to consider...";

      for (int i = 0; i < num_new_nodes; i++) {
        const tExpandCandidate& c = candidates[i];  // next candidate
        // create the corresponding a-star node and edge
        tAStarNode n;
        n.node = c.newnode;
        n.parent_link.resize(1);
        tAStarEdge *e = &(n.parent_link[0]);
        e->parent = current;
        e->edge = c.edge;
        ASSERT(c.edge_cost >= 0);  // incremental cost cannot be negative
        e->edge_cost = c.edge_cost;
        e->cumulative_cost = current->CumulativeCost() + c.edge_cost;

        n.f_value = e->cumulative_cost;
        if (is_reverse_search_)
          n.f_value += EstimateRemainingCost_Reverse(n.node);
        else
          n.f_value += EstimateRemainingCost(n.node);

        RegisterNode(n);

        // for debug -- keep track of when target nodes are seen
        if (is_oneway_search_ && IsTarget(n.node, e->cumulative_cost)) {
          if (best_cost_ < 0 || e->cumulative_cost < best_cost_) {
            VLOG(3) << (best_cost_ < 0 ?
                       "Target first generated"
                       : "Lower-cost path generated")
                   << " @ cost=" << e->cumulative_cost << "; "
                   << PrintNodeCount();
            best_cost_ = e->cumulative_cost;
          }
        }
      }
    }
  }
}

namespace astar_util {
// utility to swap two variables
template<class T>
inline void Swap(T& a, T& b) {
  T c;
  c = a;
  a = b;
  b = c;
}

// utility to reverse a vector in-place
template<class T>
inline void ReverseVector(vector<T>& v) {
  for (int i = v.size() / 2 - 1; i >= 0; i--) {
    astar_util::Swap(v[i], v[v.size() - 1 - i]);
  }
}
}

// trace a plan from the given node
template<class tNode, class tEdge, class tCost>
void AStar<tNode, tEdge, tCost>::TracePlanFromNode
                                   (const tAStarNode *final_node,
                                    vector<tPlanStep> *result) const {
  // trace up the links to build a complete plan
  result->clear();
  const tAStarNode *p = final_node;
  while (p != NULL) {
    result->resize(result->size() + 1);
    tPlanStep *step = &(result->back());
    step->node = &(p->node);
    if (p->parent_link.empty()) {
      step->earlier_cost = 0;
      step->edge_cost = 0;
      step->edge = NULL;
      break;
    }
    else {
      const tAStarEdge *e = &(p->parent_link[0]);
      step->edge = &(e->edge);
      step->earlier_cost = e->parent->CumulativeCost();
      step->edge_cost = e->edge_cost;
      p = e->parent;
    }
  }

  // "result" now contains the steps in reverse.  Now we just need to
  // reverse the vector
  astar_util::ReverseVector(*result);
}

#endif  // _PUBLIC_UTIL_ALGORITHM_ASTAR_H_
