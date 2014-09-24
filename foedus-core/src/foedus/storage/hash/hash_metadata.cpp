/*
 * Copyright (c) 2014, Hewlett-Packard Development Company, LP.
 * The license and distribution terms for this file are placed in LICENSE.txt.
 */
#include "foedus/storage/hash/hash_metadata.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "foedus/externalize/externalizable.hpp"

namespace foedus {
namespace storage {
namespace hash {
std::string HashMetadata::describe() const {
  std::stringstream o;
  o << HashMetadataSerializer(const_cast<HashMetadata*>(this));
  return o.str();
}
std::ostream& operator<<(std::ostream& o, const HashMetadata& v) {
  o << HashMetadataSerializer(const_cast<HashMetadata*>(&v));
  return o;
}

ErrorStack HashMetadataSerializer::load(tinyxml2::XMLElement* element) {
  CHECK_ERROR(load_base(element));
  CHECK_ERROR(get_element(element, "bin_bits_", &data_casted_->bin_bits_))
  return kRetOk;
}

ErrorStack HashMetadataSerializer::save(tinyxml2::XMLElement* element) const {
  CHECK_ERROR(save_base(element));
  CHECK_ERROR(add_element(element, "bin_bits_", "", data_casted_->bin_bits_));
  return kRetOk;
}

void HashMetadata::set_capacity(uint64_t expected_records, double preferred_fillfactor) {
  if (expected_records == 0) {
    expected_records = 1;
  }
  if (preferred_fillfactor >= 1) {
    preferred_fillfactor = 1;
  }
  if (preferred_fillfactor < 0.1) {
    preferred_fillfactor = 0.1;
  }
  uint64_t bin_count = expected_records / preferred_fillfactor / kMaxEntriesPerBin;
  uint8_t bits;
  for (bits = 0; bits < 64 && ((1ULL << bits) < bin_count); ++bits) {
    continue;
  }
  if (bits < 8) {
    bits = 8;
  }
  bin_bits_ = bits;
}


}  // namespace hash
}  // namespace storage
}  // namespace foedus
