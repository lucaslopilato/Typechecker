# typecheck 
Lucas Lopilato
CS160 Project4

For the typechecker, the hardest part was learning what each of the
outside classes were used for, and how the typechecker was supposed to function.

After that, I was able to create the flow of checking where if there was a 
necessary check, I performed it from the proper function. Most functions
except for code blocks also passed up a value to be evaluated further up in
the typechecker.

Another difficult part of the project involved making sure each of the nuances
of the language was carefully considered. Between the manual, piazza, and 
the project description, careful consideration was required to make sure that
each edge case was directly accounted for.

Most of my workflow involved using the AST to find where in the trace I was,
then I would pinpoint what stage of visit I would be in, and which object I was 
interacting with. From there, I used ast.hpp to find out what information I had
available to me, and used that to either interact with the symtab, recursive
basetype information, or just object information.
