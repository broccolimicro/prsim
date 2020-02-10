# prsim

A simulation environment for PRS processes.

**Usage**: `prsim [options] file...`

**Supported file formats**:
 - `*.prs`           Load an PRS
 - `*.sim`           Load a sequence of transitions to operate on

**General Options**:
 - `-h`,`--help`      Display this information
 -    `--version`   Display version information
 - `-v`,`--verbose`   Display verbose messages
 - `-d`,`--debug`     Display internal debugging messages

**Conversion Options**:


**Interactive Simulation**:
 - `<arg>` specifies a required argument
 - `(arg=value)` specifies an optional argument with a default value

**General**:
 - `help`, `h`             print this message
 - `seed <n>`            set the random seed for the simulation
 - `source <file>`       source and execute a list of commands from a file
 - `save <file>`         save the sequence of fired transitions to a '.sim' file
 - `load <file>`         load a sequence of transitions from a '.sim' file
 - `clear`, `c`            clear any stored sequence and return to random stepping
 - `quit`, `q`             exit the interactive simulation environment

**Running Simulation**:
 - `tokens`, `t`           list the location and state information of every token
 - `enabled`, `e`          return the list of enabled transitions
 - `fire <i>`, `f<i>`      fire the i'th enabled transition
 - `step (N=1)`, `s(N=1)`  step through N transitions (random unless a sequence is loaded)
 - `reset (i)`, `r(i)`     reset the simulator to the initial marking and re-seed (does not clear)

**Setting/Viewing State**:
 - `set <i> <expr>`      execute a transition as if it were local to the i'th token
 - `set <expr>`          execute a transition as if it were remote to all tokens
 - `force <expr>`        execute a transition as if it were local to all tokens

