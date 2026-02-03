define vvec
  printf "%s\n", ((facebook::velox::BaseVector*)($arg0).get())->toString((facebook::velox::vector_size_t)$arg1, (facebook::velox::vector_size_t)$arg2).c_str()
end


document vvec
Print a Velox VectorPtr slice.
Usage: vvec <expr> <start> <count>
Example: vvec b 1 5
end

