import itertools

plus = lambda x,y: x+y
minus = lambda x,y: x-y
times = lambda x,y: x*y
divided = lambda x,y: x/y

symbols = {plus: '+', minus: '-', times: '*', divided: '/'}

def generate_op_trees(inputs):
    if len(inputs) == 1:
        yield (inputs[0], str(inputs[0]))
    
    for i in range(1, len(inputs)):
        right_subtrees = list(generate_op_trees(inputs[i:]))
        left_subtrees = list(generate_op_trees(inputs[:i]))
        for left_tree in left_subtrees:
            for right_tree in right_subtrees:
                for op in [plus, minus, times, divided]:
                    if(right_tree[0]==4 and left_tree[0]==3):
                         print(f"({left_tree[1]} {symbols[op]} {right_tree[1]})" +"testé")
                    try:
                        value = op(left_tree[0], right_tree[0])
                        value = int(value) if int(value) == value else None
                    except ZeroDivisionError:
                        value = None
                    
                    if value is not None:
                        if(right_tree[0]==4 and left_tree[0]==3):
                            print(f"({left_tree[1]} {symbols[op]} {right_tree[1]}"+"yielded"+str(value))
                        yield (value, f"({left_tree[1]} {symbols[op]} {right_tree[1]})")

if __name__ == "__main__":
    numbers = [3,4,2]
    number = 10

    #for numbers_perm in itertools.permutations(numbers):
    for value, ops in generate_op_trees(numbers):
                print(f"{ops} = {value}")