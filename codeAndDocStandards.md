# Documentation and Code Standards

This document will outline the standards for documentation and code in the MonkeShell project.

# Coding Standards

## Formating

For the sake of simplicity and consistency, the coding standards will follow clang format. The clang format style can be found [here](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).

If you are using vscode here is how to install the clang format extension:


**1. Open Visual Studio Code**: <br> 
Launch the Visual Studio Code application.

**2. Access the Extensions View**: <br>
Press `Ctrl+Shift+X` to open the Extensions view.

**3. Search for Clang-Format**:  <br>
In the search bar, type "Clang-Format" and press Enter.

**4. Install Clang-Format**:  <br>
In the search results, find "Clang-Format" and click on the Install button. After installation, you may need to restart Visual Studio Code.

**5. Set Clang-Format as Default Formatter**:  <br>
After installing the extension, set Clang-Format as your default formatter.

**6. Place .clang-format File**:  <br>
Place your `.clang-format` file in the opened project directory.

**7. Update settings.json**:  <br>
Open your `settings.json` file in Visual Studio Code and add the following lines:

```json
{
  "editor.formatOnSave": true,
  "clang-format.executable": "/absolute/path/to/clang-format",
  "clang-format.language.cpp.style": "file"
}
``````
## Naming Conventions

### 1. Variables

Variables should be named using undersores. The name should be descriptive of the variable's purpose.

### 2. Functions

Functions should be named using camelCase. The name should be descriptive of the function's purpose. The only exception to this rule is if the function is meant to replace an existing standard library function. In this case, the function should be named the same as the standard library function but with `c_` prepended to the name.

### 3. Files

Files should be named using camelCase. The name should be descriptive of the file's purpose.

If the file is a test file, the name should be the same as the file it is testing with `_test` appended to the end.

### 4. Macros

Macros should be named using all caps and underscores. The name should be descriptive of the macro's purpose.

### 5. Structs

Structs should be named using underscores with capital first letter. The name should be descriptive of the struct's purpose.

### 6. Enums

Enums should be named using camelCase. The name should be descriptive of the enum's purpose.

### 7. Typedefs

Typedefs should be named using camelCase. The name should be descriptive of the type's purpose.

### 8. Defines

Defines should be named using all caps and underscores. The name should be descriptive of the define's purpose.

# Documentation

## 1. Functions

### 1.1 Function Header

The function header should be formatted as follows:

```c
/**
 * @brief A brief description of the function
 *
 * Details (optional): A detailed description of the function if needed
 * 
 * @param[?in][?out] param1 A description of the first parameter
 * .
 * .
 * .
 * @param[?in][?out] paramN A description of the Nth parameter
 * @return A description of the return value if there is one
 */
```

### 1.2 Function Body & Inline Comments

Documentation here is not as strict as the function header. The function body should be documented as needed to explain the code. Inline comments should be used to explain any complex code or code that is not self-explanatory.

## 2. Files

### 2.1 Header File

The top of every header file should be formatted as follows:

```c
/**
 * @file filename.c
 * @brief A brief description of the file
 * @authors 
 * 
 * Details: A detailed description of the file
 * 
 * Assumptions/Limitations: Any assumptions or limitations of the code
 * 
 * @date 2021-03-01
 */
``` 
### 2.2 Source File

The top of every source file should be formatted as follows:

```c
/**
 * @file filename.c
 * @authors
 * 
 * @date
 */
```

## 3. Makefiles

Makefiles simply require the author and date to be included at the top of the file.

```make
# Author(s):
# Date:
```




