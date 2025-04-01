import math

def output(r1, r2, pot, pot_ratio):
    r1_t = r1+(pot*pot_ratio)
    r2_t = r2+(pot*(1-pot_ratio))
    
    vout = 0.6 / (r2_t / (r1_t+r2_t))
    return vout


r1 = 6800
r2 = 1000
pot = 25000

print(output(r1,r2,pot, 0))
print(output(r1,r2,pot, 0.5))
print(output(r1,r2,pot, 1.0))
