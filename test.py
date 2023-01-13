def function(input: str) -> str:
    result = ''
    for i in range(len(input)):
        if i % 2 == 0:
            result = result + input[i]
    return result


print(function('test'))