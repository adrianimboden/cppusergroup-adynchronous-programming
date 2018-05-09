lambda { |stdout,stderr,status|
  output = stdout + stderr
  return :red   if /\[  FAILED  \]/.match(output)
  return :green if /\[  PASSED  \]/.match(output)
  return :amber
}
