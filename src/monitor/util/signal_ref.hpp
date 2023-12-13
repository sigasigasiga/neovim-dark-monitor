#pragma once

namespace monitor::util {

/// \brief `boost::signals2::signal`'s interface is way too powerful,
///        we need to cut it down
template <typename... Args> class signal_ref_t {
public:
  // signals that return some value are disaster
  using signature_t = void(Args...);

  using connection_t = boost::signals2::scoped_connection;
  using slot_t = boost::signals2::slot<signature_t>;
  using sig_t = boost::signals2::signal<signature_t>;

public:
  // this constructor is implicit intentionally, such that it would be easier
  // to return signal ref from a function
  signal_ref_t(sig_t &sig) : sig_{sig} {}

public:
  connection_t subscribe(const slot_t &slot) { return sig_.connect(slot); }

private:
  sig_t &sig_;
};

} // namespace monitor::util
