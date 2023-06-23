from dataclasses import dataclass

@dataclass
class TestCaseItem:
    id: int
    img_x: float
    img_y: float
    mag: float

def get_sort_key(elem):
    return elem.mag

count = 1
fp_input = open('input.csv')
fp_output = open('output.csv')

# go through each line in the input csv file
for line in fp_input:
    print("Test Case: {}".format(str(count)))
    # split each line by the comma delimiter
    split_line = line.strip().split(",")

    # Create a list to store the matching between the inputs and outputs
    items = list()

    # Get the matching output line to this input line
    output_item = fp_output.readline()
    split_output= output_item.strip().split(",")

    # the input item counter
    i = 0
    # the output item counter
    j = 0
    while i < len(split_line):
        # Read three items from the input and one from the output
        # print("{} {} {} {}".format(str(i), str(j), str(len(split_line)), str(len(split_output))))
        new_item = TestCaseItem(split_output[j], split_line[i+0],\
           split_line[i+1], split_line[i+2])
        # new_item = TestCaseItem(split_output[j], 0, 0, 0)
        items.append(new_item)
        #print("{} {} {}".format(str(split_line[i+0]), str(split_line[i+1]), str(split_line[i+2])))
        i = i + 3
        j = j + 1

    # sort by star brightness
    items.sort(key=get_sort_key)

    i = 0
    # output the result
    for sorted_item in items:
        print("{} {} {} {}".format(str(sorted_item.id), str(sorted_item.img_x), \
            str(sorted_item.img_y), str(sorted_item.mag)))
        i = i + 1

        if i == 15:
            break

    count = count + 1
