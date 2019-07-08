#include "pch.h"
#include "DbResult.h"
#include "MariaResultPrep.h"
#include "MariaResultSimple.h"



// Construction ////////////////////////////////////////////////////////////////

DbResult::DbResult(const DbConnectionPtr& pConn) :
  pConn_(pConn)
{
  pConn_->check_connection();
  pConn_->set_current_result(this);
}

DbResult::~DbResult() {
  try {
    pConn_->set_current_result(NULL);
  } catch (...) {};
}


// Publics /////////////////////////////////////////////////////////////////////

bool DbResult::complete() const {
  return (impl == NULL) || impl->complete();
}

bool DbResult::is_active() const {
  return pConn_->is_current_result(this);
}

int DbResult::n_rows_fetched() {
  return impl->n_rows_fetched();
}

int DbResult::n_rows_affected() {
  return impl->n_rows_affected();
}

void DbResult::bind(const List& params) {
  validate_params(params);
  impl->bind(params);
}

List DbResult::fetch(const int n_max) {
  if (!is_active())
    stop("Inactive result set");

  return impl->fetch(n_max);
}

List DbResult::get_column_info() {
  List out = impl->get_column_info();

  out.attr("row.names") = IntegerVector::create(NA_INTEGER, -Rf_length(out[0]));
  out.attr("class") = "data.frame";

  return out;
}

void DbResult::close() {
  // Called from destructor
  if (impl) impl->close();
}

// Privates ///////////////////////////////////////////////////////////////////

void DbResult::validate_params(const List& params) const {
  if (params.size() != 0) {
    SEXP first_col = params[0];
    int n = Rf_length(first_col);

    for (int j = 1; j < params.size(); ++j) {
      SEXP col = params[j];
      if (Rf_length(col) != n)
        stop("Parameter %i does not have length %d.", j + 1, n);
    }
  }
}
