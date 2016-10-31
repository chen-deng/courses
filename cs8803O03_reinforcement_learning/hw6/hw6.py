def value(g, rA1, rA2, rB, p_):
    expectedA = p_ * rA1 + (1 - p_) * rA2
    expectedB = rB
    optimal = max(expectedA, expectedB)
    long_term_value = optimal / (1.0 - g)
    return long_term_value

if __name__ == "__main__":
    gamma=0.34062644
    rewardA1=909.25566594
    rewardA2=-416.84698504
    rewardB=-612.56812480
    p=0.49390717
    print value(gamma, rewardA1, rewardA2, rewardB, p)
    