import winsound
import time
print("Testing Speaker... You should hear 3 beeps.")
for _ in range(3):
    winsound.Beep(2000, 200) # High pitch, short duration
    time.sleep(0.5)
print("Test Complete.")