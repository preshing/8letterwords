import random
import sys

random.seed()
allWords = [w.strip() for w in open('8letterwords.txt')]
sample = random.sample(allWords, 100)
for i in xrange(10000000):
    sys.stdout.write(random.choice(sample))
    sys.stdout.write('\n' if i % 10 == 9 else ' ')
