// fsm_pkg.sv - shared types for the FSM, its testbench and (later) the UVM scoreboard
package fsm_pkg;
    typedef enum logic [1:0] {IDLE, RUNNING, PAUSED, FAULT} state_t;
endpackage
