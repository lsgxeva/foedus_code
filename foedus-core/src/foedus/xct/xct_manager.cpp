/*
 * Copyright (c) 2014, Hewlett-Packard Development Company, LP.
 * The license and distribution terms for this file are placed in LICENSE.txt.
 */
#include <foedus/xct/xct_manager.hpp>
#include <foedus/xct/xct_manager_pimpl.hpp>
namespace foedus {
namespace xct {
XctManager::XctManager(Engine* engine) : pimpl_(nullptr) {
  pimpl_ = new XctManagerPimpl(engine);
}
XctManager::~XctManager() {
  delete pimpl_;
  pimpl_ = nullptr;
}

ErrorStack  XctManager::initialize() { return pimpl_->initialize(); }
bool        XctManager::is_initialized() const { return pimpl_->is_initialized(); }
ErrorStack  XctManager::uninitialize() { return pimpl_->uninitialize(); }

Epoch       XctManager::get_current_global_epoch() const {
  return pimpl_->get_current_global_epoch();
}
Epoch       XctManager::get_current_global_epoch_weak() const {
  return pimpl_->get_current_global_epoch_weak();
}

void        XctManager::advance_current_global_epoch() { pimpl_->advance_current_global_epoch(); }
ErrorStack  XctManager::wait_for_commit(Epoch commit_epoch, int64_t wait_microseconds) {
  return pimpl_->wait_for_commit(commit_epoch, wait_microseconds);
}

ErrorStack  XctManager::begin_xct(thread::Thread* context, IsolationLevel isolation_level) {
  return pimpl_->begin_xct(context, isolation_level);
}
ErrorStack XctManager::begin_schema_xct(thread::Thread* context) {
  return pimpl_->begin_schema_xct(context);
}

ErrorStack  XctManager::precommit_xct(thread::Thread* context, Epoch *commit_epoch) {
  return pimpl_->precommit_xct(context, commit_epoch);
}
ErrorStack  XctManager::abort_xct(thread::Thread* context)  { return pimpl_->abort_xct(context); }

}  // namespace xct
}  // namespace foedus
