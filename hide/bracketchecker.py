import os
import glob


for filename in glob.iglob('C:/Users/Trace/BWAPI/FrogFish/Source/' + '**/*', recursive=True):
    if filename.endswith('.cpp') or filename.endswith('.h'):
        print(f"\n{filename}")
        with open(filename) as cppfile:
            file_lines = cppfile.readlines()
            line_numbers = []
            stack = []

            for i in range(len(file_lines)):
                for c in file_lines[i]:
                    if c == '[' or c == '{' or c == '(':
                        stack.append(c)
                        line_numbers.append(i)
                    elif c == ']':
                        if stack[-1] != '[':
                            print(f'improper close square at line {i}')
                        else:
                            stack = stack[:-1]
                            line_numbers = line_numbers[:-1]
                    elif c == '}':
                        if stack[-1] != '{':
                            print(f'improper close curly at line {i}')
                        else:
                            stack = stack[:-1]
                            line_numbers = line_numbers[:-1]
                    elif c == ')':
                        if stack[-1] != '(':
                            print(f'improper close curved at line {i}')
                        else:
                            stack = stack[:-1]
                            line_numbers = line_numbers[:-1]
            if len(stack) > 0:
                for i in range(len(stack)):
                    print(f'dangling {stack[i]} at line {line_numbers[i]}')
