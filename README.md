An interactive proof-of-concept assembly REPL that executes user-supplied machine code by switching full CPU contexts.
# Runtime / Code State Transition:
```
{runtime state 0}
save runtime state 0:
    after %rsp is saved:
        pushfq
        movq (%rsp), %rax
        movq %rax, (code_state.flags)
        popfq
generate code
{runtime state 1}
restore code state
{code state 0}
push code_state.rip(%rip)
ret
{code state 0} ------------------------------------> {code state 0}
                                                     do something
                                                     {code state 1}
                                                     push %rdi //save %rdi
                                                     movabs &code_state into %rdi
                                                     save code state 1 except for %rdi:
                                                        after %rsp is saved:
                                                            pushfq
                                                            movq (%rsp), %rax
                                                            movq %rax, (code_state.flags)
                                                            add $8, (code_state.rsp)
                                                            popfq
                                                        pop (code_state.rdi) //save %rdi
                                                     {code state 2}
                                                     movabs runtime_state into %rdi
                                                     restore runtime state 0 except for the flags, %rsp, %rax and %rdi
                                                     lea -8(%rsp), %rsp
                                                     movq (runtime_state.flags), %rax
                                                     movq %rax, (%rsp)
                                                     popfq
                                                     movq (runtime_state.rsp), %rsp
                                                     movq (runtime_state.rax), %rax
                                                     push runtime_state.rip(%rip)
                                                     movq (runtime_state.rdi), %rdi
                                                     {runtime state 0}
{runtime state 0}<---------------------------------- ret 
```