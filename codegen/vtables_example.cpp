// VTables Implementation Example for Hulk Compiler
// This demonstrates how virtual method dispatch works with your implementation

/*
EXAMPLE HULK CODE THAT NOW SUPPORTS POLYMORPHISM:

type Animal {
    name: String := "Unknown";
    
    speak(): String {
        return "Some generic animal sound";
    }
    
    getName(): String {
        return self.name;
    }
}

type Dog inherits Animal {
    breed: String := "Mixed";
    
    speak(): String {  // This OVERRIDES Animal.speak()
        return "Woof! Woof!";
    }
    
    getBreed(): String {
        return self.breed;
    }
}

type Cat inherits Animal {
    color: String := "Black";
    
    speak(): String {  // This OVERRIDES Animal.speak()
        return "Meow! Meow!";
    }
}

function main(): Number {
    // Polymorphic variables - base type can hold derived instances
    let animal1: Animal := new Dog();
    let animal2: Animal := new Cat();
    
    // Virtual method calls - will call the correct overridden method
    print(animal1.speak());  // Prints: "Woof! Woof!" (Dog's implementation)
    print(animal2.speak());  // Prints: "Meow! Meow!" (Cat's implementation)
    
    // Inherited methods work too
    print(animal1.getName()); // Works because Dog inherits getName from Animal
    
    return 0;
}

HOW THE VTABLES WORK:

1. ANIMAL VTABLE:
   [0] speak -> Animal_speak
   [1] getName -> Animal_getName

2. DOG VTABLE:
   [0] speak -> Dog_speak       (OVERRIDDEN - different function!)
   [1] getName -> Animal_getName (INHERITED - same function)
   [2] getBreed -> Dog_getBreed (NEW method)

3. CAT VTABLE:
   [0] speak -> Cat_speak       (OVERRIDDEN - different function!)  
   [1] getName -> Animal_getName (INHERITED - same function)

4. GENERATED LLVM FUNCTIONS:
   - Animal_speak() -> returns "Some generic animal sound"
   - Animal_getName() -> returns self.name
   - Dog_speak() -> returns "Woof! Woof!"
   - Dog_getBreed() -> returns self.breed  
   - Cat_speak() -> returns "Meow! Meow!"

5. VIRTUAL DISPATCH:
   When calling animal1.speak():
   - Compiler knows animal1 is declared as Animal
   - But at runtime, animal1 actually contains a Dog instance
   - Method call resolves to Dog_speak() through vtable lookup
   - Result: polymorphic behavior!

WHAT YOUR IMPLEMENTATION DOES:

1. During TypeDeclNode processing:
   - Creates vtables with method pointers for each type
   - Handles inheritance by copying parent methods and overriding with child methods
   - Assigns method indices for fast lookup

2. During method calls (MethodMember):
   - Uses callVirtualMethod() to find the correct method implementation
   - Dispatches to the most derived version of the method
   - Falls back to direct resolution if needed

3. Key Features:
   - ✅ All methods are virtual by default (as requested)
   - ✅ Method overriding works correctly
   - ✅ Inheritance of non-overridden methods
   - ✅ Polymorphic behavior through base class references
   - ✅ Clean, KISS implementation as requested

BENEFITS:
- True object-oriented polymorphism
- Runtime method resolution
- Proper inheritance hierarchies
- Maintainable code structure
- Extensible for future OOP features

This implementation provides the foundation for full OOP support in your Hulk language!
*/
