# forward_command_controller: commands go to the wrong joints if the interfaces arrive in another order

`ForwardControllersBase::update()` writes `data[i]` of the command to
`command_interfaces_[i]`, so it assumes the loaned command interfaces are in
the order of the `joints` parameter. `on_activate()` already builds the
interfaces in that order with `get_ordered_interfaces()`, but only checks that
all of them are present and then discards the result (see the ATTENTION
comment there).

When the interfaces are assigned in a different order, for example by a test
fixture or a resource manager that sorts them, every command lands on the
wrong joint without any error.

## Steps to reproduce

1. Configure the controller with joints `j0, j1, j2`.
2. Assign the command interfaces in the order `j2, j1, j0` and activate.
3. Publish `[0.0, 1.0, 2.0]`.

## Expected

`j0` = 0.0, `j1` = 1.0, `j2` = 2.0.

## Actual

`j0` = 2.0, `j1` = 1.0, `j2` = 0.0.
