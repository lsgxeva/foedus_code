/*
 * Copyright (c) 2014, Hewlett-Packard Development Company, LP.
 * The license and distribution terms for this file are placed in LICENSE.txt.
 */
#ifndef FOEDUS_XCT_XCT_MANAGER_HPP_
#define FOEDUS_XCT_XCT_MANAGER_HPP_
#include <foedus/fwd.hpp>
#include <foedus/initializable.hpp>
#include <foedus/xct/fwd.hpp>
#include <foedus/thread/fwd.hpp>
namespace foedus {
namespace xct {
/**
 * @brief Xct Manager class that provides API to begin/end transaction.
 * @ingroup XCT
 * @details
 * For client programs, this class provides only 3 interesting
 */
class XctManager CXX11_FINAL : public virtual Initializable {
 public:
    explicit XctManager(Engine* engine);
    ~XctManager();

    // Disable default constructors
    XctManager() CXX11_FUNC_DELETE;
    XctManager(const XctManager&) CXX11_FUNC_DELETE;
    XctManager& operator=(const XctManager&) CXX11_FUNC_DELETE;

    ErrorStack  initialize() CXX11_OVERRIDE;
    bool        is_initialized() const CXX11_OVERRIDE;
    ErrorStack  uninitialize() CXX11_OVERRIDE;

    /**
     * @brief Returns the current global epoch.
     * @param[in] fence_before_get Whether to take memory fence before obtaining the epoch.
     */
    Epoch       get_global_epoch(bool fence_before_get = true) const;

    /**
     * @brief Begins a new transaction on the thread.
     * @param[in,out] context Thread context
     * @pre context->is_running_xct() == false
     */
    ErrorStack  begin_xct(thread::Thread* context);

    /**
     * @brief Prepares the currently running transaction on the thread for commit.
     * @pre context->is_running_xct() == true
     * @param[in,out] context Thread context
     * @param[out] commit_epoch When successfully prepared, this value indicates the commit
     * epoch of the prepared transaction. When the global epoch reaches this value, the
     * transaction is deemed as committed.
     * @details
     * As the name of this method implies, this method is \b NOT a commit yet.
     * The transaction is deemed as committed only when the global epoch advances.
     * This method merely \e prepares this transaction to be committed so that the caller
     * can move on to other transactions in the meantime.
     */
    ErrorStack  prepare_commit_xct(thread::Thread* context, Epoch *commit_epoch);

    /**
     * @brief Aborts the currently running transaction on the thread.
     * @param[in,out] context Thread context
     * @pre context->is_running_xct() == true
     */
    ErrorStack  abort_xct(thread::Thread* context);

 private:
    XctManagerPimpl *pimpl_;
};
}  // namespace xct
}  // namespace foedus
#endif  // FOEDUS_XCT_XCT_MANAGER_HPP_