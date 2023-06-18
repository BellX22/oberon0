# Oberon-0
A C Implementation of Niklaus Wirth's Oberon-0 Language from the book 
'Compiler Construction'. It writes 3-address-codes to the console. 
But it would be easy to implement a generator for x64-assembly.

# Grammar
```cpp
Identifier = letter { letter | digit }
Integer = digit { digit }
Number = Integer

Selector          = { "." Identifier | "[" Expression "]" }
Factor            = Identifier Selector
                    | Number
                    | "(" Expression ")"
                    | "~" Factor
Term              = Factor { ("*" | "div" | "mod" | "&") Factor }
SimpleExpression  = [ "+" | "-" ] Term { ("+" | "-" | "or") Term }
Expression        = SimpleExpression [ ("=" | "#" | "<" | "<=" | ">" | ">=") SimpleExpression ]
Assignment        = Identifier Selector ":=" Expression
IfStatement       = "if" Expression "then" StatementSequence 
                    { "elsif" Expression "then" StatementSequence } 
                    [ "else" StatementSequence ] 
                    "end if"
WhileStatement    = "while" Expression "do" StatementSequence "end"
ActualParameters  = "(" Expression { "," Expression } ")"
ProcedureCall     = Identifier [ ActualParameters ]
Statement         = [ Assignment 
                    | IfStatement 
                    | WhileStatement 
                    | RepeatStatement 
                    | ProcedureCall ]
StatementSequence = Statement { ";" Statement }
IdentList         = Identifier { "," Identifier }
ArrayType         = "array" Expression "of" Type
FieldList         = [ IdentList ":" Type ]
RecordType        = "record" FieldList { ";" FieldList } "end"
Type              = Identifier 
                    | ArrayType 
                    | RecordType
FpSection         = [ "var" ] IdentList ":" Type
FormalParameters  = "(" [ FpSection { ";" FpSection}] ")"
ProcedureHeading  = "procedure" Identifier [ FormalParameters ]
ProcedureBody     = Declarations [ "begin" StatementSequence ] "end" Identifier
ProcedureDeclaration = ProcedureHeading ";" ProcedureBody
Declarations      = [ "const" { Identifier "=" Expression ";" ]
                    [ "type" { Identifier "=" Type ";" } ]
                    [ "var" { Identifier ":" Type ";" } ]
                    { ProcedureDeclaration ";" }
Module = "module" Identifier ";" Declarations
```
