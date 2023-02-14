from dataclasses import dataclass

@dataclass
class TestCaseItem:
    img_x: float
    img_y: float
    mag: float

def get_sort_key(elem):
    return elem.mag

num_inputs = len(open('input.csv').readlines())
count = 0

print("StarCentroids testInputs[{}] = {}".format(str(num_inputs), "{"))

fp_input = open('input.csv')

# go through each line in the input csv file
for line in fp_input:
    #print("line {}".format(line))
    # split each line by the comma delimiter
    split_line = line.strip().split(",")

    # Create a list to store the inputs so we can sort them
    items = list()

    # the input item counter
    i = 0
    while i < len(split_line):
        # Read three items from the input and output it in the C++ format
        new_item = TestCaseItem(split_line[i+0],\
            split_line[i+1], split_line[i+2])
        items.append(new_item)
        i = i + 3

    # sort by star brightness
    items.sort(key=get_sort_key)

    print("StarCentroids{}".format("{"))

    added_items = 0

    # output the results from the list
    for sorted_item in items:
        print("StarCentroid{{{},{},{}}}".format(str(sorted_item.img_x), \
            str(sorted_item.img_y), str(sorted_item.mag)), end = "")

        added_items = added_items + 1

        if (added_items == 15):
            break
        else:
            print(",")

    # now output any extra items if required
    for i in range(added_items, 15):
        print("StarCentroid{}0.0,0.0,0.0{}".format("{", "}"), end = "")

        added_items = added_items + 1

        if (added_items == 15):
            break
        else:
            print(",")

    count = count + 1

    print("{}".format("}"), end = "")
    if (count < num_inputs):
        print(",")

print("{};".format("}"))

fp_input.close()
