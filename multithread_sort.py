import heapq
import random
import threading
import time
import sys

ARRAY_SIZE = int(sys.argv[1])
PARALLEL_THREADS_NUMBER = int(sys.argv[2])
INT_MIN = -2**31 - 1
INT_MAX = 2**31

def GenerateBigArray(array_size):
    result_array = []
    for i in range(array_size):
        result_array.append(random.randint(INT_MIN, INT_MAX))
    
    return result_array

def sort(x):
    return x.sort()

def sort_multithreaded(array, parallel_threads_number):
    part_size = int(len(array) / parallel_threads_number)
    start_ind = 0
    array_parts = []
    for i in range(parallel_threads_number):
        array_parts.append(array[start_ind : min(len(array), start_ind + part_size)])
        start_ind += part_size
    
    threads_array = []
    for array_part in array_parts:
        threads_array.append(threading.Thread(target=sort, args=([array_part])))
        threads_array[-1].start()
    
    for thread in threads_array:
        thread.join()
    
    heap = []

    for i, array_part in enumerate(array_parts):
        heapq.heappush(heap, (array_part[0], i, 0))

    result_array = []
    while not heap:
        (value, i, j) = heapq.heappop(heap)
        result_array.append(value)
        if j != len(array_parts[i]) - 1:
            heapq.heappush(heap, (array_part[j + 1], i, j + 1))
    
    return result_array

big_array = GenerateBigArray(ARRAY_SIZE)

array_copy_1 = big_array.copy()
array_copy_2 = big_array.copy()

print("Array size: %s, threads_number: %s" % (ARRAY_SIZE, PARALLEL_THREADS_NUMBER))
start_time = time.time()
array_copy_1.sort()
print("one-threaded sort: %s seconds" % (time.time() - start_time))
# print(array_copy_1)

start_time = time.time()
sort_multithreaded(array_copy_2, PARALLEL_THREADS_NUMBER)
print("multithreaded sort: %s seconds" % (time.time() - start_time))
# print(array_copy_2)
