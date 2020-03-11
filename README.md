## Circuit Analysis

The task for this coursework was to write a C program that will analyse general electrical circuits connected in the common cascade connection as set by [EE20084] (http://www.bath.ac.uk/catalogues/2018-2019/ee/EE20084.html): *Structured Programming*. Series and shunt resistances of any value can be connected in any order between the source and load of the circuit. The general program flow is as follows:

•	The circuits source information, each resistors nodal connections and their values were read from a given input. file. The required   outputs and units were also extracted whilst conducting error checks to ensure the data validity.

•	Data structures containing each resistors information was sorted using the quicksort algorithm, in order from the source to the       load.

•	Each resistors equivalent ABCD matrix was created and then multiplied together, in order from the source to the load, to calculate   the circuits equivalent impedance.

•	This impedance was then used to calculate the required outputs and write them to the output file in the required order with           appropriate units.

