def value(n, B_, MAX_AMT_):
  if n >= MAX_AMT_:
    return n

  vn = 0
  for i, b in enumerate(B_):
    if b == 0:
      vn += value(n + i + 1, B_, MAX_AMT_)
  return max(n, vn / float(len(B_)))

if __name__ == "__main__":
  DICE = "10101110"
  B = [int(f) for f in DICE]
  MAX_AMT = 25
  print "B = {%s}\nvalue = %f" % (B, value(0, B, MAX_AMT))