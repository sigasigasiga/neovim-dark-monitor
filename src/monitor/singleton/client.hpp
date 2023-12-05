#pragma once

namespace monitor::singleton {

class client_t {
public:
  class state_dumper_t {
  public:
    virtual ~state_dumper_t() = default;
    virtual msgpack::sbuffer dump_state() = 0;
  };

public:
  client_t(state_dumper_t &state_dumper,
           boost::asio::generic::stream_protocol::socket socket);

public:
  void start();

private:
  state_dumper_t &state_dumper_;
  boost::asio::generic::stream_protocol::socket socket_;
  msgpack::sbuffer buffer_;
};

} // namespace monitor::singleton
