# Explanation of `FACTORIAL` in Forth

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

This process computes the factorial of a number using only stack operations, 
showcasing the power and efficiency of Forth's concise structure.