import gdb

class VeloxVec(gdb.Command):
    """
    vvec EXPR [START] [COUNT]
    Example:
      vvec a 0 10
      vvec dow 1 2
    Prints a Velox vector slice using BaseVector::toString(start,count).
    """

    def __init__(self):
        super(VeloxVec, self).__init__("vvec", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        if len(argv) < 1:
            print("usage: vvec EXPR [START] [COUNT]")
            return

        expr = argv[0]
        start = int(argv[1]) if len(argv) >= 2 else 0
        count = int(argv[2]) if len(argv) >= 3 else 10

        # Force gdb to call through BaseVector* to avoid overload resolution issues.
        # Also force vector_size_t types explicitly.
        cmd = (
            f"set $vv = ((facebook::velox::BaseVector*)({expr}.get()))"
            f"->toString((facebook::velox::vector_size_t){start},"
            f" (facebook::velox::vector_size_t){count})"
        )
        gdb.execute(cmd)
        gdb.execute("x/s $vv.c_str()")

VeloxVec()
print("Loaded Velox gdb helpers: use `vvec <expr> [start] [count]`")
