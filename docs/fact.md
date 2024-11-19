# Explanation of `FACTORIAL` in Forth

This is one of the tests for Forth.

This code defines a word (function) called `FACTORIAL` in **Forth**, 
which calculates the factorial of a number. Here’s a detailed explanation:

## Word Definition

- `: FACTORIAL ( +n1 -- +n2 )`:
    - This line starts the definition of the `FACTORIAL` word.
    - The comment in parentheses indicates that the word takes one positive number (`+n1`) 
    - from the stack and returns one positive number (`+n2`) to the stack.

## Initial Duplication and Comparison

- `DUP 2 < IF DROP 1 EXIT THEN`:
    - `DUP`: Duplicate the number on the top of the stack.
    - `2 <`: Check if the duplicated number is less than 2.
    - `IF DROP 1 EXIT THEN`: If the top of the stack (duplicated number)
    - is less than 2, then drop that number, push 1 onto the stack, and exit the `FACTORIAL` 
    - word (since fact(1) and fact(0) are 1).

## Preparing for the Loop

- `DUP`: Again, duplicate the number on the top of the stack.

## Loop Setup

- `BEGIN DUP 2 > WHILE`:
    - Begin a loop and continue while the number on the top of the stack is greater than 2.
        - `DUP 2 >`: Duplicate the number and compare it to 2.
        - `WHILE`: Continue the loop while the condition (`DUP 2 >`) is true.

## Loop Body

- `1- SWAP OVER * SWAP`:
    - `1-`: Decrement the top number of the stack by 1.
    - `SWAP`: Swap the top two numbers on the stack.
    - `OVER`: Copy the second item on the stack and push it on top.
    - `*`: Multiply the top two numbers on the stack.
    - `SWAP`: Swap the top two numbers, restoring the original order for the next iteration.

## Loop Termination

- `REPEAT DROP`: End the loop. After the loop finishes, drop the last number, which should be 1.

## End of Definition

- `;`: End the definition of the `FACTORIAL` word.

## Example Walkthrough

For instance, calculating the factorial of 5 (`5` on the stack):

- Initial Stack: [5]
- **First Iteration**:
    - After `DUP`: [5, 5]
    - `DUP 2 <` fails.
    - `DUP`: [5, 5, 5]
    - `BEGIN DUP 2 > WHILE` succeeds.
- **Inside Loop**:
    - `1-`: [5, 5, 4]
    - `SWAP`: [5, 4, 5]
    - `OVER`: [5, 4, 5, 4]
    - `*`: [5, 4, 20]
    - `SWAP`: [5, 20, 4]
- Repeat till `2 >` test fails.

Finally, when it exits:
- `DROP`: Remaining stack: [120] (which is `5!`)




## Recursive version `rfact` Word Definition and Explanation

### Word Definition

```forth
: rfact ( +n1 -- +n2 ) 
    DUP 2 < 
    IF DROP 1 EXIT THEN  
    DUP 1- RECURSE * 
;
```

### Explanation

This definition uses several Forth words and constructs to achieve a recursive factorial calculation. Here's a step-by-step breakdown:

1. `: rfact ( +n1 -- +n2 )`
  - This defines a new word named `rfact`. The comment `(+n1 -- +n2)` indicates that it takes a single positive number as input (`+n1`) and produces a single positive number as output (`+n2`).

2. `DUP`
  - This duplicates the top stack item. After this operation, the stack has the same number twice, e.g., if the stack was `[5]`, it becomes `[5 5]`.

3. `2 <`
  - This compares the top stack item with `2`. If the item is less than `2`, it leaves a true flag (`-1`), otherwise, it leaves a false flag (`0`).

4. `IF`
  - This starts a conditional block. If the top of the stack is true (non-zero), it will execute the code following `IF`.

5. `DROP`
  - This drops the top item on the stack. If the 
  - comparison earlier was true, we drop the number as we don't need it anymore.

6. `1`
  - This pushes `1` onto the stack. This is the base 
  - case for the recursion (i.e., `0! = 1` and `1! = 1`).

7. `EXIT`
  - This exits the word immediately, effectively 
  - returning `1` for any input less than `2`.

8. `THEN`
  - This marks the end of the conditional block.

9. `DUP`
  - Again, duplicate the top item on the stack 
  - because we need it for the multiplication after the recursive call.

10. `1-`
  - This decrements the top stack item by `1`.

11. `RECURSE`
  - This word calls `rfact` recursively.

12. `*`
  - This multiplies the top two items on the stack.

### Full Documentation

```forth
: rfact ( +n1 -- +n2 )
    \ Start by duplicating the input number
    DUP 
    \ Check if the number is less than 2
    2 < 
    IF 
        \ If the number is 1 or 0, it's the base case
        \ Drop the number (since we don't need it anymore)
        DROP 
        \ Push 1 onto the stack, as 0! and 1! are both 1
        1 
        \ Exit the word
        EXIT 
    THEN  
    \ Duplicate the number for multiplication later
    DUP
    \ Decrement the number by 1
    1- 
    \ Recursively call rfact with the decremented number
    RECURSE 
    \ Multiply the result of the recursive call with the current number
    * 
;
```

### Example Usage

To calculate the factorial of 5, you would use:

```forth
5 rfact .
```

This would push the result (120) to the stack and
print it.

### Summary

The `rfact` word implements factorial calculation
using recursion. It checks if the input number is less than 2, in which case it returns 1. If the number is 2 or greater, it recursively calls itself with the number decremented by 1 and then multiplies the result with the original number.