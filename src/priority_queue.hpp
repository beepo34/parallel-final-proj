/******************************************************************************
 * fifo_node_bucket_pq.h
 *
 * Taken from source of VieCut: https://github.com/VieCut/VieCut/blob/4aaaddbaf147dc529216e4302e994b92057c92a5/lib/data_structure/priority_queues/fifo_node_bucket_pq.h
 *
 ******************************************************************************
 * Copyright (C) 2013-2015 Christian Schulz <christian.schulz@kit.edu>
 * Copyright (C) 2018 Alexander Noe <alexander.noe@univie.ac.at>
 *
 * Published under the MIT license in the LICENSE file.
 *****************************************************************************/

 #pragma once

 #include <deque>
 #include <limits>
 #include <unordered_map>
 #include <utility>
 #include <vector>
 
 #include "definitions.h"
 
 class fifo_node_bucket_pq {
  public:
     fifo_node_bucket_pq(const NodeID& num_nodes, const EdgeWeight& gain_span);
 
     virtual ~fifo_node_bucket_pq() { }
 
     NodeID size();
     void insert(NodeID id, Gain gain);
     bool empty();
 
     Gain maxValue();
     NodeID maxElement();
     NodeID deleteMax();
 
     void decreaseKey(NodeID node, Gain newGain);
     void increaseKey(NodeID node, Gain newGain);
 
     void changeKey(NodeID element, Gain newKey);
     Gain getKey(NodeID element);
     void deleteNode(NodeID node);
 
     bool contains(NodeID node);
     Gain gain(NodeID Node);
 
  private:
     NodeID m_elements;
     EdgeWeight m_gain_span;
     unsigned m_max_idx;
 
     std::vector<std::pair<Count, Gain> > m_queue_index;
     std::vector<std::deque<NodeID> > m_buckets;
     std::vector<size_t> m_bucket_offset;
 };
 
 inline fifo_node_bucket_pq::fifo_node_bucket_pq(
     const NodeID& num_nodes, const EdgeWeight& gain_span_input)
     : m_bucket_offset(2 * gain_span_input + 1, 0) {
     m_elements = 0;
     m_gain_span = gain_span_input;
     m_max_idx = 0;
 
     m_buckets.resize(2 * m_gain_span + 1);
     m_queue_index.resize(num_nodes, std::make_pair(
                              static_cast<Count>(UNDEFINED_COUNT),
                              static_cast<Gain>(0)));
 }
 
 inline NodeID fifo_node_bucket_pq::size() {
     return m_elements;
 }
 
 inline void fifo_node_bucket_pq::insert(NodeID node, Gain gain) {
     unsigned address = gain + m_gain_span;
     if (address > m_max_idx) {
         m_max_idx = address;
     }
 
     m_buckets[address].push_back(node);
     m_queue_index[node].first = m_buckets[address].size()
                                 + m_bucket_offset[address] - 1;
 
     m_queue_index[node].second = gain;
 
     m_elements++;
 }
 
 inline bool fifo_node_bucket_pq::empty() {
     return m_elements == 0;
 }
 
 inline Gain fifo_node_bucket_pq::maxValue() {
     return m_max_idx - m_gain_span;
 }
 
 inline NodeID fifo_node_bucket_pq::maxElement() {
     return m_buckets[m_max_idx].front();
 }
 
 inline NodeID fifo_node_bucket_pq::deleteMax() {
     NodeID node = m_buckets[m_max_idx].front();
    //  VIECUT_ASSERT_TRUE(m_queue_index[node].first != UNDEFINED_COUNT);
    UPCXX_ASSERT(m_queue_index[node].first != UNDEFINED_COUNT);
 
     m_bucket_offset[m_max_idx]++;
     m_buckets[m_max_idx].pop_front();
     // m_queue_index.erase(node);
     m_queue_index[node].first = UNDEFINED_COUNT;
 
     if (m_buckets[m_max_idx].size() == 0) {
         // update max_idx
         while (m_max_idx != 0) {
             m_max_idx--;
             if (m_buckets[m_max_idx].size() > 0) {
                 break;
             }
         }
     }
 
     m_elements--;
     return node;
 }
 
 inline void fifo_node_bucket_pq::decreaseKey(NodeID node, Gain new_gain) {
     changeKey(node, new_gain);
 }
 
 inline void fifo_node_bucket_pq::increaseKey(NodeID node, Gain new_gain) {
     changeKey(node, new_gain);
 }
 
 inline Gain fifo_node_bucket_pq::getKey(NodeID node) {
     return m_queue_index[node].second;
 }
 
 inline void fifo_node_bucket_pq::changeKey(NodeID node, Gain new_gain) {
     deleteNode(node);
     insert(node, new_gain);
 }
 
 inline void fifo_node_bucket_pq::deleteNode(NodeID node) {
    //  VIECUT_ASSERT_TRUE(m_queue_index[node].first != UNDEFINED_COUNT);
    UPCXX_ASSERT(m_queue_index[node].first != UNDEFINED_COUNT);
 
     Gain old_gain = m_queue_index[node].second;
     unsigned address = old_gain + m_gain_span;
     Count in_bucket_idx = m_queue_index[node].first - m_bucket_offset[address];
     m_bucket_offset[address]++;
 
     if (m_buckets[address].size() > 1) {
         // swap current element with last element and pop_back
         m_queue_index[m_buckets[address].front()].first =
             m_queue_index[node].first;
         std::swap(m_buckets[address][in_bucket_idx],
                   m_buckets[address].front());
         m_buckets[address].pop_front();
     } else {
         // size is 1
         m_buckets[address].pop_front();
         if (address == m_max_idx) {
             // update max_idx
             while (m_max_idx != 0) {
                 m_max_idx--;
                 if (m_buckets[m_max_idx].size() > 0) {
                     break;
                 }
             }
         }
     }
 
     m_elements--;
     m_queue_index[node].first = UNDEFINED_COUNT;
 }
 
 inline bool fifo_node_bucket_pq::contains(NodeID node) {
     return m_queue_index[node].first != UNDEFINED_COUNT;
 }
 
 inline Gain fifo_node_bucket_pq::gain(NodeID node) {
     auto it_node = m_queue_index[node];
 
     if (it_node.first == UNDEFINED_COUNT) {
         return 0;
     } else {
         return it_node.second;
     }
 }