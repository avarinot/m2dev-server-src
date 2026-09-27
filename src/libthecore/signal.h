#pragma once
void signal_setup();
void signal_timer_disable();
void signal_timer_enable(int timeout_seconds);
void signal_poll_shutdown();