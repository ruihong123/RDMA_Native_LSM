// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A filter block is stored near the end of a Table file.  It contains
// filters (e.g., bloom filters) for all data blocks in the table combined
// into a single filter block.

#ifndef STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "leveldb/slice.h"
#include "leveldb/options.h"
#include "util/hash.h"

#include "block_builder.h"

namespace leveldb {

class FilterPolicy;
class Env;
class Options;
// A FilterBlockBuilder is used to construct all of the filters for a
// particular Table.  It generates a single string which is stored as
// a special block in the Table.
//
// The sequence of calls to FilterBlockBuilder must match the regexp:
//      (StartBlock AddKey*)* Finish
class FilterBlockBuilder {
 public:
  explicit FilterBlockBuilder(const FilterPolicy* policy, ibv_mr* mr,
                              std::map<int, ibv_mr*>* remote_mrs,
                              std::shared_ptr<RDMA_Manager> rdma_mg);

  FilterBlockBuilder(const FilterBlockBuilder&) = delete;
  FilterBlockBuilder& operator=(const FilterBlockBuilder&) = delete;

  void StartBlock(uint64_t block_offset);
  size_t CurrentSizeEstimate();
  void AddKey(const Slice& key);
  Slice Finish();
  void Reset();
  void Flush();
  Slice result;           // Filter data computed so far

 private:
  void GenerateFilter();

  const FilterPolicy* policy_;
  std::shared_ptr<RDMA_Manager> rdma_mg_;
  ibv_mr* local_mr;
  std::map<int, ibv_mr*>* remote_mrs_;
  std::string keys_;             // Flattened key contents
  std::vector<size_t> start_;    // Starting index in keys_ of each key
  //todo Make result Slice; make Policy->CreateFilter accept Slice rather than string
  std::vector<Slice> tmp_keys_;  // policy_->CreateFilter() argument
  std::vector<uint32_t> filter_offsets_;
};

class FilterBlockReader {
 public:
  // REQUIRES: "contents" and *policy must stay live while *this is live.
  FilterBlockReader(const FilterPolicy* policy, const Slice& contents);
  ~FilterBlockReader();
  bool KeyMayMatch(uint64_t block_offset, const Slice& key);

 private:
  const FilterPolicy* policy_;
  const char* data_;    // Pointer to filter data (at block-start)
  const char* offset_;  // Pointer to beginning of offset array (at block-end)
  size_t num_;          // Number of entries in offset array
  size_t base_lg_;      // Encoding parameter (see kFilterBaseLg in .cc file)
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_
