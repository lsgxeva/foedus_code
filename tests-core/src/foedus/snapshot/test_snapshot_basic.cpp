/*
 * Copyright (c) 2014, Hewlett-Packard Development Company, LP.
 * The license and distribution terms for this file are placed in LICENSE.txt.
 */
#include <gtest/gtest.h>

#include <string>

#include "foedus/engine.hpp"
#include "foedus/engine_options.hpp"
#include "foedus/test_common.hpp"
#include "foedus/log/log_manager.hpp"
#include "foedus/snapshot/snapshot_id.hpp"
#include "foedus/snapshot/snapshot_manager.hpp"
#include "foedus/snapshot/snapshot_manager_pimpl.hpp"
#include "foedus/snapshot/snapshot_metadata.hpp"
#include "foedus/storage/storage_manager.hpp"
#include "foedus/storage/array/array_metadata.hpp"
#include "foedus/storage/array/array_storage.hpp"
#include "foedus/storage/array/fwd.hpp"
#include "foedus/xct/xct_manager.hpp"

/**
 * @file test_snapshot_basic.cpp
 * Easy testcases for snapshotting.
 * These don't involve data, multiple snapshots, etc etc.
 */
namespace foedus {
namespace snapshot {
DEFINE_TEST_CASE_PACKAGE(SnapshotBasicTest, foedus.snapshot);

ErrorStack read_metadata_file(Engine* engine, SnapshotMetadata *metadata) {
  SnapshotManager& manager = engine->get_snapshot_manager();
  SnapshotId snapshot_id = manager.get_previous_snapshot_id();
  EXPECT_NE(kNullSnapshotId, snapshot_id);
  fs::Path file = manager.get_pimpl()->get_snapshot_metadata_file_path(snapshot_id);
  CHECK_ERROR(metadata->load_from_file(file));
  return kRetOk;
}

TEST(SnapshotBasicTest, Empty) {
  EngineOptions options = get_tiny_options();
  Engine engine(options);
  COERCE_ERROR(engine.initialize());
  {
    UninitializeGuard guard(&engine);
    engine.get_snapshot_manager().trigger_snapshot_immediate(true);

    SnapshotMetadata metadata;
    COERCE_ERROR(read_metadata_file(&engine, &metadata));
    EXPECT_EQ(engine.get_snapshot_manager().get_previous_snapshot_id(), metadata.id_);
    EXPECT_EQ(Epoch::kEpochInvalid, metadata.base_epoch_);
    EXPECT_NE(Epoch::kEpochInvalid, metadata.valid_until_epoch_);
    EXPECT_EQ(0, metadata.largest_storage_id_);

    COERCE_ERROR(engine.uninitialize());
  }
  cleanup_test(options);
}

TEST(SnapshotBasicTest, OneArrayCreate) {
  EngineOptions options = get_tiny_options();
  Engine engine(options);
  COERCE_ERROR(engine.initialize());
  {
    UninitializeGuard guard(&engine);
    storage::array::ArrayStorage out;
    Epoch commit_epoch;
    storage::array::ArrayMetadata meta("test", 16, 100);
    COERCE_ERROR(engine.get_storage_manager().create_array(&meta, &out, &commit_epoch));
    EXPECT_TRUE(out.exists());
    EXPECT_TRUE(commit_epoch.is_valid());
    COERCE_ERROR(engine.get_xct_manager().wait_for_commit(commit_epoch));
    engine.get_snapshot_manager().trigger_snapshot_immediate(true);

    SnapshotMetadata metadata;
    COERCE_ERROR(read_metadata_file(&engine, &metadata));
    EXPECT_EQ(engine.get_snapshot_manager().get_previous_snapshot_id(), metadata.id_);
    EXPECT_EQ(Epoch::kEpochInvalid, metadata.base_epoch_);
    EXPECT_GE(Epoch(metadata.valid_until_epoch_), commit_epoch);
    EXPECT_EQ(1U, metadata.largest_storage_id_);

    storage::array::ArrayMetadata *array = reinterpret_cast<storage::array::ArrayMetadata*>(
      metadata.get_metadata(out.get_id()));
    EXPECT_EQ(out.get_id(), array->id_);
    EXPECT_EQ(out.get_name(), array->name_);
    EXPECT_EQ(out.get_type(), array->type_);
    EXPECT_EQ(out.get_array_size(), array->array_size_);
    EXPECT_EQ(out.get_payload_size(), array->payload_size_);

    COERCE_ERROR(engine.uninitialize());
  }
  cleanup_test(options);
}

TEST(SnapshotBasicTest, TwoArrayCreate) {
  EngineOptions options = get_tiny_options();
  Engine engine(options);
  COERCE_ERROR(engine.initialize());
  {
    UninitializeGuard guard(&engine);
    storage::array::ArrayStorage out;
    Epoch commit_epoch;
    storage::array::ArrayMetadata meta("test", 16, 10);
    COERCE_ERROR(engine.get_storage_manager().create_array(&meta, &out, &commit_epoch));
    EXPECT_TRUE(out.exists());
    storage::array::ArrayStorage out2;
    storage::array::ArrayMetadata meta2("test2", 50, 20);
    COERCE_ERROR(engine.get_storage_manager().create_array(&meta2, &out2, &commit_epoch));
    EXPECT_TRUE(out2.exists());

    EXPECT_TRUE(commit_epoch.is_valid());
    COERCE_ERROR(engine.get_xct_manager().wait_for_commit(commit_epoch));
    engine.get_snapshot_manager().trigger_snapshot_immediate(true);

    SnapshotMetadata metadata;
    COERCE_ERROR(read_metadata_file(&engine, &metadata));
    EXPECT_EQ(engine.get_snapshot_manager().get_previous_snapshot_id(), metadata.id_);
    EXPECT_EQ(Epoch::kEpochInvalid, metadata.base_epoch_);
    EXPECT_GE(Epoch(metadata.valid_until_epoch_), commit_epoch);
    EXPECT_EQ(2U, metadata.largest_storage_id_);

    {
      storage::array::ArrayMetadata* array = reinterpret_cast<storage::array::ArrayMetadata*>(
        metadata.get_metadata(out.get_id()));
      EXPECT_EQ(out.get_id(), array->id_);
      EXPECT_EQ(out.get_name(), array->name_);
      EXPECT_EQ(out.get_type(), array->type_);
      EXPECT_EQ(out.get_array_size(), array->array_size_);
      EXPECT_EQ(out.get_payload_size(), array->payload_size_);
    }

    {
      storage::array::ArrayMetadata* array = reinterpret_cast<storage::array::ArrayMetadata*>(
        metadata.get_metadata(out2.get_id()));
      EXPECT_EQ(out2.get_id(), array->id_);
      EXPECT_EQ(out2.get_name(), array->name_);
      EXPECT_EQ(out2.get_type(), array->type_);
      EXPECT_EQ(out2.get_array_size(), array->array_size_);
      EXPECT_EQ(out2.get_payload_size(), array->payload_size_);
    }

    COERCE_ERROR(engine.uninitialize());
  }
  cleanup_test(options);
}

}  // namespace snapshot
}  // namespace foedus
