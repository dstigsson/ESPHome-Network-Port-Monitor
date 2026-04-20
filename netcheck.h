#pragma once

#include "esphome.h"
#include "esphome/components/socket/socket.h"

#include <esp_timer.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>

namespace netmon {

struct ProbeResult {
  bool ok{false};
  int latency_ms{-1};
  std::string error{"unknown"};
};

inline ProbeResult tcp_probe(const std::string &ip, uint16_t port, uint32_t timeout_ms = 1000) {
  ProbeResult out;

  auto sock = esphome::socket::socket_ip(SOCK_STREAM, 0);
  if (!sock) {
    out.error = "socket_create_failed";
    return out;
  }

  struct sockaddr_storage addr {};
  auto addrlen = esphome::socket::set_sockaddr(
      reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr), ip, port);
  if (addrlen == 0) {
    out.error = "invalid_ip";
    return out;
  }

  sock->setblocking(false);

  const int64_t start_us = esp_timer_get_time();
  int rc = sock->connect(reinterpret_cast<struct sockaddr *>(&addr), addrlen);

  if (rc == 0) {
    out.ok = true;
    out.latency_ms = static_cast<int>((esp_timer_get_time() - start_us) / 1000);
    out.error = "";
    sock->close();
    return out;
  }

  if (errno != EINPROGRESS && errno != EWOULDBLOCK && errno != EALREADY) {
    out.error = "connect_failed";
    sock->close();
    return out;
  }

  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(sock->get_fd(), &wfds);

  struct timeval tv {};
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  rc = select(sock->get_fd() + 1, nullptr, &wfds, nullptr, &tv);

  if (rc == 1 && FD_ISSET(sock->get_fd(), &wfds)) {
    int so_error = 0;
    socklen_t optlen = sizeof(so_error);
    if (sock->getsockopt(SOL_SOCKET, SO_ERROR, &so_error, &optlen) == 0 && so_error == 0) {
      out.ok = true;
      out.latency_ms = static_cast<int>((esp_timer_get_time() - start_us) / 1000);
      out.error = "";
    } else {
      out.error = "closed_or_refused";
    }
  } else if (rc == 0) {
    out.error = "timeout";
  } else {
    out.error = "select_error";
  }

  sock->close();
  return out;
}

}  // namespace netmon
