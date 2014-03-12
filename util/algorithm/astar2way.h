#ifndef _UTIL_ASTAR2WAY_H_
#define _UTIL_ASTAR2WAY_H_

#include "astar.h"

// two-way A* search algorithm (based on AStar class)

template<class AStarBase>
class AStarTwoWay {
 public:
  AStarTwoWay() {};
  virtual ~AStarTwoWay() {};

  typedef typename AStarBase::BaseCost tCost;
  typedef typename AStarBase::BaseEdge tEdge;
  typedef typename AStarBase::BaseNode tNode;
  typedef typename AStarBase::tAStarNode tAStarNode;
  typedef typename AStarBase::tAStarEdge tAStarEdge;
  typedef typename AStarBase::tPlanStep tPlanStep;


  // define source node
  inline void AddSrcNode(const tNode& src) {
    forward_search_.AddSrcNode(src);
    reverse_search_.AddDestNode(src);
  }

  // define destination node
  inline void AddDestNode(const tNode& dest) {
    forward_search_.AddDestNode(dest);
    reverse_search_.AddSrcNode(dest);
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
  virtual tCost Search(vector<tPlanStep> *result);

  inline AStarBase *forward_search() { return &forward_search_; }
  inline AStarBase *reverse_search() { return &reverse_search_; }

  inline int NumForwardOpenNodes() const {
    return forward_search_.NumOpenNodes();
  }
  inline int NumForwardClosedNodes() const {
    return forward_search_.NumClosedNodes();
  }
  inline int NumReverseOpenNodes() const {
    return reverse_search_.NumOpenNodes();
  }
  inline int NumReverseClosedNodes() const {
    return reverse_search_.NumClosedNodes();
  }

  // debug output: give open/closed node count information
  string PrintNodeCount() const {
    stringstream ss;
    ss << "forward: open " << NumForwardOpenNodes()
       << " / closed " << NumForwardClosedNodes()
       << "; reverse: open " << NumReverseOpenNodes()
       << " / closed " << NumReverseClosedNodes();
    return ss.str();
  }

 protected:
  // check if the next node to expand (in either direction) has been
  // encountered in the other search, and if so, add a connector node
  // to connect the two sides (even though we already have a complete
  // path here, we cannot declare success, because the path is not
  // necessarily the shortest one)
  bool CheckForConnector(AStarBase& this_dir, AStarBase& other_dir);

  // check if the next expansion candidate is a special connector node
  // (indicating success)
  bool CheckSuccess(const AStarBase& this_dir, const AStarBase& other_dir,
                    vector<tPlanStep> *result) const;

  // reverse steps in vector<tPlanStep> structure
  void ReversePlanSteps(vector<tPlanStep>& plan) const;

  // two AStarBase objects performing search in two directions
  AStarBase forward_search_;
  AStarBase reverse_search_;

  tCost best_cost_;
};


// implementations

// perform actual two-way search
template<class AStarBase>
typename AStarBase::BaseCost AStarTwoWay<AStarBase>::Search(vector<tPlanStep>
                                                            *result) {
  bool searching = true;
  tCost best_cost = -1;

  result->clear();

  // set directional flags for search objects
  forward_search_.SetForwardSearch();
  reverse_search_.SetReverseSearch();

  ASSERT(NumForwardClosedNodes() == 0 && NumReverseClosedNodes() == 0)
    << ("Each A-star object can only be used once.  "
        "Be sure to destroy the object and re-construct a new one "
        "before starting another search.");

  ASSERT(result != NULL);

  while (searching) {
    const tAStarNode *forward_candidate = forward_search_.FirstOpenNode();
    const tAStarNode *reverse_candidate = reverse_search_.FirstOpenNode();

    //
    // decide whether we should expand a forward_search node or a
    // reverse_search node in this turn (pick the one with smaller f_value)
    //
    AStarBase *this_search = NULL;
    AStarBase *other_search = NULL;
    if (forward_candidate == NULL) {
      if (reverse_candidate == NULL) {
        searching = false;  // search has failed
        break;
      }
      else {
        // expand reverse_search_ in this turn
        this_search = &reverse_search_;
        other_search = &forward_search_;
      }
    }
    else {
      if (reverse_candidate == NULL) {
        // expand forward_search_ in this turn
        this_search = &forward_search_;
        other_search = &reverse_search_;
      }
      else {
        // Both forward_candidate and reverse_candidate exist.  Pick the one
        // with smaller f_value
        if (forward_candidate->f_value <= reverse_candidate->f_value) {
          // expand forward_search_ in this turn
          this_search = &forward_search_;
          other_search = &reverse_search_;
        }
        else {
          // expand reverse_search_ in this turn
          this_search = &reverse_search_;
          other_search = &forward_search_;
        }
      }
    }

    // check for success node (i.e., connector node)
    if (CheckSuccess(*this_search, *other_search, result)) {
      // successful

      // reverse plan steps, if necessary
      if (this_search == &reverse_search_)
        ReversePlanSteps(*result);  // plan steps are reversed

      best_cost = result->back().CumulativeCost();
      searching = false;
    }
    else {
      // check if the next node to expand (in either direction) has been
      // encountered in the other search
      if (!CheckForConnector(*this_search, *other_search)) {
        // In this search, the top node is not in the other search's
        // closed list.  Proceed to regular search.
        this_search->SearchOneStep();
        searching =
          (forward_search_.searching() || reverse_search_.searching());
      }
    }
  }

  return best_cost;
}


// check if the next node to expand (in either direction) has been
// encountered in the other search
template<class AStarBase>
bool AStarTwoWay<AStarBase>::CheckForConnector(AStarBase& this_dir,
                                               AStarBase& other_dir) {
  const tAStarNode *expand_candidate = this_dir.FirstOpenNode();
  if (expand_candidate == NULL) return false;

  const tAStarNode *other_side = other_dir.FindInClosedList(expand_candidate);

  if (other_side != NULL) {
    // a complete path has been found (but it is not necessarily the best
    // path).

    // move the expansion candidate to closed list, and create a connector
    // node to connect with the other side

    this_dir.MoveFirstOpenNodeToClosedList();

    // trace plan for other side, in order to find target node
    // (note that multiple target nodes are allowed, so we must
    // trace plan to determine which one)
    vector<tPlanStep> other_side_plan;
    other_dir.TracePlanFromNode(other_side, &other_side_plan);
    ASSERT(!other_side_plan.empty());
    const tNode *target = other_side_plan[0].node;

    // create the corresponding a-star node and edge
    tAStarNode n;
    n.twoway_connector = true;  // special flag indicating connector node
    n.node = *target;
    n.parent_link.resize(1);

    tAStarEdge *e = &(n.parent_link[0]);
    e->parent = expand_candidate;
    // e->edge is undefined
    e->edge_cost = other_side->CumulativeCost();
    e->cumulative_cost = expand_candidate->CumulativeCost() + e->edge_cost;

    n.f_value = e->cumulative_cost;

    this_dir.RegisterNode(n);

    if (best_cost_ < 0 || e->cumulative_cost < best_cost_ ) {
      VLOG(2) << (best_cost_ < 0 ?
                 "First connector node created"
                 : "Lower-cost connector node created")
             << (&this_dir == &forward_search_ ?
                 " (forward search)" : " (reverse search)")
             << " @ cost=" << expand_candidate->CumulativeCost()
             << "+" << other_side->CumulativeCost()
             << "=" << e->cumulative_cost << "; "
             << PrintNodeCount();
      best_cost_ = e->cumulative_cost;
    }

    VLOG(3) << "    node content: "
           << serial::Serializer::ToJSON(expand_candidate->node);

    return true;
  }
  else
    return false;  // the two searches have not yet met
}


// check if the next expansion candidate is a special connector node
// (indicating success)
template<class AStarBase>
bool AStarTwoWay<AStarBase>::CheckSuccess(const AStarBase& this_dir,
                                          const AStarBase& other_dir,
                                          vector<tPlanStep> *result) const {
  const tAStarNode *expand_candidate = this_dir.FirstOpenNode();
  if (expand_candidate == NULL) return false;

  if (expand_candidate->twoway_connector) {
    ASSERT(expand_candidate->parent_link.size() == 1);
    const tAStarNode *common_node = expand_candidate->parent_link[0].parent;

    VLOG(2) << "Success: cost=" << expand_candidate->CumulativeCost();

    // the next expansion candidate is a special connector
    // -- we have found the shortest path.
    // -- the path can be constructed by concatenating paths from
    //    both directions
    const tAStarNode *other_side =
      other_dir.FindInClosedList(common_node);  // this must exist
    ASSERT(other_side != NULL);

    // trace plan for this side
    this_dir.TracePlanFromNode(common_node, result);
    ASSERT(!result->empty());

    // trace plan for other side
    vector<tPlanStep> other_side_plan;
    other_dir.TracePlanFromNode(other_side, &other_side_plan);
    ASSERT(!other_side_plan.empty());

    // // debug
    // LOG(INFO) << "Dumping plan from one side";
    // AStarBase::DumpPlan(*result);
    // LOG(INFO) << "Dumping plan from the other side";
    // AStarBase::DumpPlan(other_side_plan);

    // concatenate the plans together
    int next = result->size();
    tCost cumulative_cost = result->back().CumulativeCost();
    result->resize(result->size() + (other_side_plan.size() - 1));
    for (int i = other_side_plan.size() - 1; i > 0; i--) {
      tPlanStep& step = (*result)[next];
      step.edge = other_side_plan[i].edge;
      step.earlier_cost = cumulative_cost;
      step.edge_cost = other_side_plan[i].edge_cost;
      step.node = other_side_plan[i - 1].node;  // node is off by 1 step

      cumulative_cost += step.edge_cost;
      ++next;
    }
    return true;
  }
  else
    return false;
}


// reverse steps in vector<tPlanStep> structure
template<class AStarBase>
void AStarTwoWay<AStarBase>::ReversePlanSteps(vector<tPlanStep>& plan) const {
  VLOG(4) << "Reversing plan steps...";

  // first, reverse the vector in-place
  astar_util::ReverseVector(plan);

  // Now "node" field is correct, "edge" and "edge_cost" are off by 1 step,
  // and earlier_cost is totally wrong.  We fix them here.

  // fix edge and edge_cost
  ASSERT(plan.back().edge == NULL);
  for (int i = plan.size() - 1; i > 0; i--) {
    plan[i].edge = plan[i - 1].edge;
    plan[i].edge_cost = plan[i - 1].edge_cost;
  }
  plan[0].edge = NULL;
  plan[0].edge_cost = 0;

  // fix earlier_cost field
  tCost cumulative_cost = 0;
  for (int i = 0; i < plan.size(); i++) {
    plan[i].earlier_cost = cumulative_cost;
    cumulative_cost += plan[i].edge_cost;
  }

  // done!
}

#endif
