/*
 * Ticker-tape serializer RPC definitions
 */

typedef string msg_t<79>;

struct submit_args {
  msg_t msg;
};
struct submit_result {
  bool ok;
};
struct xaction_args{
 int my_id;
 msg_t msg;
 int ts;
};

program TICKER_PROG {
  version TICKER_VERS {
    submit_result TICKER_SUBMIT (submit_args) = 1;
    submit_result TICKER_XACTION(xaction_args) = 2;
  } = 1;
} = 400001;
