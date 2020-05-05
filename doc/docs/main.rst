Name
----
|bv| - static C++-compiler-based checking tool

Synopsis
--------
|bv| ``[options...] [compiler options...] file...``

Usage Examples
--------------

|bv| my_comp.cpp
    Run |bv| using all defaults.  This will use a set of -D and -I directives
    appropriate to the Bloomberg environment, and a configuration file that has
    most checks enabled.

|bv| -D EXPERT=0 -I common my_comp.cpp
    Run |bv| using all defaults, and also include custom macro definitions and
    include paths.

|bv| -nodefdef -D EXPERT=1 -nodefinc -I common -I local my_comp.cpp
    Run |bv| using no default macros or include paths, supplying our own.  The
    default configuration file is still used.  (The |BV| macro is always
    defined.)

|bv| -p build_dir my_comp.cpp
    Run |bv| using no default macros or include paths, extracting appropriate
    ones from a compilation database file named ``compile_commands.json`` that
    must be present in the specified build_dir directory.

|bv| -config my_bv.cfg my_comp.cpp
    Run |bv| using default macros and paths, but with a custom configuration
    file.

|bv| -cl 'all off' -cl 'check longlines on' -cl 'check headline on' my_comp.cpp
    Run |bv| using defaults, with all but the 'longlines' and 'headline' checks
    disabled.

|bv| -cl 'append dictionary presquash untreeify' my_comp.cpp
    Run |bv| using defaults, modifying the ``dictionary`` configuration
    setting (used by the spelling checker) to include extra words.

|bv| -cl 'all off' -cl 'check headline on' -rewrite-dir . my_comp.cpp
    Run |bv| using defaults, with all but the 'headline' checks disabled.
    If the first line of the file is malformed, produce a corrected version
    named my_comp.cpp-rewritten in the current directory.

Description
-----------
The |bv| command performs a variety of checks intended to foster improved
adherence to |BDE| design rules, coding standards, and practices and to detect
potential errors. It is built within the **clang** C++ compiler tool system,
and therefore has access to proper syntax and type information about the
program it is examining, unlike text-based scanning tools. The tool was
originally developed (as ``coolyser``) by `Dietmar Kuhl`_ and is now being
maintained and enhanced by `Hyman Rosen`_. This is the `original project`_ and
this is the `original list of implemented checks`_. (You may need to request
permission from Dietmar to visit those pages.)

.. _Hyman Rosen: hrosen4@bloomberg.net
.. _Dietmar Kuhl: dkuhl@bloomberg.net
.. _original project: https://github.com/dietmarkuehl/coolyser
.. _original list of implemented checks:
   https://github.com/dietmarkuehl/coolyser/wiki/Overview

If the ``-rewrite-dir dir`` option is specified, |bv| will make some suggested
changes by itself and place the modified files in the specified directory with
``-rewritten`` appended to the file names. (Not much rewriting is being done
yet; we plan to increase this over time.)

|Bv| is supported on SunOS, Linux, and Windows. In the BloombergLP development
environment, running ``/opt/bb/bin/`` |bv| will launch the appropriate
version. 

|Bv| now contains the experimental feature of automatically rewriting a class
to be allocator-aware.  To use it, enable the ``allocator-forward`` check, set
the ``allocator_transform`` configuration variable to contain the name(s) of
the classes to be modfied, and enable rewriting as specified above.  The
rewritten class does not yet fully comply with the BDE coding standard (|bv|
will complain!)

Options and Configuration
-------------------------

|Bv| reads a configuration file to determine which checks it should run and to
set values of parameters that affect some checks.  That file is distributed
along with |bv| (in the installation as ``.../etc/bde-verify/bde_verify.cfg``.)

You may prepare your own customized copy and use the ``-config`` option to use
that file instead of the standard one.  Additionally, you may specify the
option ``-cl 'line'`` multiple times, and |bv| will treat those lines as if
they were appended to the configuration file.  This is often used to have |bv|
perform a single check, as in

    |bv| ``-cl 'all off'`` ``-cl 'check longlines on'`` file.cpp

Details of the configuration file contents are described below.

Compilation Options and Databases
---------------------------------

Invoking |bv| is similar to invoking the compiler.  The same ``-I`` and ``-D``
options must be provided so that the program is compiled with appropriate
header files and macro definitions, and the 32/64-bit and C++ standard mode
specifications will affect the results.  In the Bloomberg environment, |bv|
attempts to help by providing default macro definitions and include paths that
point to installed headers.  However, it is likely that users of |bv| will want
full control over macros and include paths.  The ``-nodefdef`` option prevents
|bv| from providing its own macro definitions and the ``-nodefinc`` option
prevents |bv| from supplying its own include paths.  Users can specify those
options and then provide exactly the ``-D`` and ``-I`` parameters needed.

In addition, |bv| can extract these options from a compilation database.  A
compilation databse is a file named ``compile_commands.json`` that is produced
as an artifact of compilation by many build systems.  |Bv| accepts a
``-p directory`` option where the specified directory is the one containing
the compilation database.  When a file to be processed is found in the
compilation database, |bv| will extract the ``-D`` and ``-I`` paramaters from
the database and apply them to its run.  (In this case, |bv| will act as if
``-nodefdef`` and ``-nodefinc`` were the defaults.)

Here is an example of building a component in the Bloomberg development
environment from the BDE code base and then running |bv| using the resulting
compilation database::

    # Check out build tools and a source tree
    % git clone bbgithub:bde/bde-tools
    % git clone bbgithub:bde/bde
    % cd bde

    # Configure, build, and run one test program
    % eval $(../bde-tools/bin/bde_build_env.py)
    % ../bde-tools/bin/cmake_build.py configure build \
      --tests run --targets bdlb_randomdevice

    # Run bde_verify using the resulting compilation database
    % bde_verify \
      -p $(dirname $(find _build/ -name compile_commands.json)) \
      groups/bdl/bdlb/bdlb_randomdevice.h

As of this writing, that produces output from the pedantic "that-which"
grammar check but nothing else::

    groups/bdl/bdlb/bdlb_randomdevice.h:17:62: warning: TW01: Possibly prefer 'that' over 'which'
    // random number generators.  Two variants are provided: one which may block,
                                                                 ^
    groups/bdl/bdlb/bdlb_randomdevice.h:19:4: warning: TW01: Possibly prefer 'that' over 'which'
    // which does not block, but which potentially should not be used for
       ^
    groups/bdl/bdlb/bdlb_randomdevice.h:39:54: warning: TW01: Possibly prefer 'that' over 'which'
    // both available and leaving it for users to decide which to use.
                                                         ^
    3 warnings generated.

Without the compilation database, we would need to specify the include paths::

    % /opt/bb/bin/bde_verify -nodefdef -nodefinc \
      -I groups/bsl/bsls -I groups/bsl/bslscm -I groups/bdl/bdlscm \
      groups/bdl/bdlb/bdlb_randomdevice.h

Or we could let |bv| use the installed versions of the headers instead of the
local ones, and simply run::

    % bde_verify groups/bdl/bdlb/bdlb_randomdevice.h

Command-line Options
--------------------

+-----------------------+-----------------------------------------------------+
| Parameter             | Description                                         |
+=======================+=====================================================+
| **Pass-Through**      |                                                     |
| **Options**           |                                                     |
+-----------------------+-----------------------------------------------------+
| -D\ *macro*           | Define *macro* for the compilation.                 |
+-----------------------+-----------------------------------------------------+
| -I\ *directory*       | Add *directory* to the include path.                |
+-----------------------+-----------------------------------------------------+
| -W\ *warning*         | Enable the specified compiler *warning*.            |
+-----------------------+-----------------------------------------------------+
| -f\ *flag*            | Pass the specifed *flag* through to the compiler.   |
|                       | This is for often-specified compiler options such   |
|                       | as -fexceptions.                                    |
+-----------------------+-----------------------------------------------------+
| | -m32                | Process in 32-bit or 64-bit mode.                   |
| | -m64                |                                                     |
+-----------------------+-----------------------------------------------------+
| -std *type*           | Specify C++ version as *type*.                      |
+-----------------------+-----------------------------------------------------+
| -w                    | Disable normal compiler warnings (but not |bv|      |
|                       | warnings).                                          |
+-----------------------+-----------------------------------------------------+
| -\ *misc*             | Various ignored compiler options, e.g., -pipe.      |
+-----------------------+-----------------------------------------------------+
| **Paths and**         |                                                     |
| **Directories**       |                                                     |
+-----------------------+-----------------------------------------------------+
| -bb *directory*       | Specify the trunk *directory* where Bloomberg       |
|                       | software is installed.  |Bv| will add directories   |
|                       | to the include path from here unless -nodefinc is   |
|                       | is specified.                                       |
+-----------------------+-----------------------------------------------------+
| -cc *compiler*        | Specify the full path of a g++ or clang++ compiler. |
|                       | |Bv| will use this to find system and               |
|                       | compiler-dependent header files.  This defaults to  |
|                       | the value of the CXX environment variable if        |
|                       | present, and a compiler found in the shell path     |
|                       | otherwise.  Typically specify the same compiler     |
|                       | used in the build.                                  |
+-----------------------+-----------------------------------------------------+
| -exe *program*        | Specify the underlying executable file that |bv|    |
|                       | will invoke (usually when testing a new version).   |
+-----------------------+-----------------------------------------------------+
| **Operation**         |                                                     |
+-----------------------+-----------------------------------------------------+
| -config *file*        | Specify the *file* containing |bv| configuration    |
|                       | options.  (The file format is described below.)     |
+-----------------------+-----------------------------------------------------+
| -cl *'line'*          | Specify an additional configuration *line* (may be  |
|                       | repeated multiple times).  These lines are treated  |
|                       | as if they were appended to the configuration file. |
+-----------------------+-----------------------------------------------------+
| -[no]defdef           | [Do not] set up default macro definitions.          |
|                       | However, |BV| is always defined.                    |
+-----------------------+-----------------------------------------------------+
| -[no]definc           | [Do not] use default include paths.                 |
+-----------------------+-----------------------------------------------------+
| -[no]ovr              | | [Un]define ``BSL_OVERRIDES_STD``.                 |
|                       | | This macro is deprecated, so the default is       |
|                       |   -noovr.                                           |
+-----------------------+-----------------------------------------------------+
| -diff *file*          | Specify a *file* (use ``-`` for standard input) in  |
|                       | diff format (such as might be produced by running   |
|                       | ``git diff``).  |Bv| output will be restricted to   |
|                       | only those lines that are marked as changed.        |
|                       |                                                     |
|                       | Reading standard input facilitiates piping:         |
|                       |                                                     |
|                       | | git diff | |bv| -diff - file.cpp                  |
+-----------------------+-----------------------------------------------------+
| -p *directory*        | Specify a *directory* containg a file named         |
|                       | ``compile_commands.json``.  |Bv| will look there    |
|                       | for build lines for the files it is processing and  |
|                       | use -D and -I options it finds.  (Use -nodefdef and |
|                       | -nodefinc to avoid mixing in default values.)       |
|                       | Such "compilation database" files are produced by   |
|                       | many build systems.                                 |
+-----------------------+-----------------------------------------------------+
| | -rewrite-dir        | Certain |bv| checks can create modified files       |
|   *directory*         | that contain suggested changes.  These files are    |
| | -rewrite            | created with the name *file*-\ ``rewritten`` in the |
|   *directory*         | given *directory* if this option is specified.  If  |
| | -rd *directory*     | this option is not specified, no rewritten files    |
|                       | are created.                                        |
+-----------------------+-----------------------------------------------------+
| | -rewrite-file       | Certain |bv| checks can create modified files       |
|   *file*              | that contain suggested changes.  If this option is  |
| | -rf *file*          | specified, a cumulative database of changes to be   |
|                       | made is kept in *file* (and maintained across       |
|                       | multiple runs of |bv|).  Those changed files are    |
|                       | created once |bv| is run with the -rd option.       |
|                       | (This option is generally not used.)                |
+-----------------------+-----------------------------------------------------+
| -diagnose *type*      | Limit files for which |bv| warnings will appear:    |
|                       |                                                     |
|                       | | ``main``        - Specified file only.            |
|                       | | ``component``   - Specified file and its .h file. |
|                       | | ``nogen``       - Skip auto-generated files.      |
|                       | | ``all``         - All included header files.      |
|                       |                                                     |
|                       | The default is ``component``.  Use ``main`` if you  |
|                       | plan to run |bv| on .h and .cpp files separately.   |
+-----------------------+-----------------------------------------------------+
| **Miscellaneous**     |                                                     |
+-----------------------+-----------------------------------------------------+
| -debug                | Output a very noisy representation of the program   |
|                       | while processing it, meant for |bv| developers.     |
+-----------------------+-----------------------------------------------------+
| -[no]nsa              | [Do not] allow logging of |bv| command lines for    |
|                       | purposes of tracking and evaluating usage.          |
+-----------------------+-----------------------------------------------------+
| -tag *string*         | Include *[string]* in |bv| messages, to distinguish |
|                       | them from compiler messages.                        |
+-----------------------+-----------------------------------------------------+
| | -verbose            | Display the full command line passed to the         |
| | -v                  | underlying executable program.  Note that options   |
|                       | from the compilation database are read by that      |
|                       | program and so will not appear here.                |
+-----------------------+-----------------------------------------------------+
| -version              | Display the version number of |bv| and of the Clang |
|                       | compiler it is based upon.                          |
+-----------------------+-----------------------------------------------------+
| | -help               | Display this usage information.                     |
| | -?                  |                                                     |
+-----------------------+-----------------------------------------------------+

Git-Diff Output Restriction
---------------------------
The output of |bv| can be restricted to include only those warnings whose line
numbers fall within a set of changes given by the output of a ``git diff``
command.  Such output contains lines beginning with ``+++`` representing a file
with changes and lines starting with ``@`` and containing ``+LINE_NUMBER`` or
``+LINE_NUMBER,NUMBER_OF_LINES`` representing which lines in the file have
changed.  Such diffs may be saved in a file and given to |bv| via the option
``-diff file`` or they may be piped into |bv| via the option ``-diff -`` in
which case standard input will be read for the diffs.

Note that the file names upon which |bv| will operate must still be specified
on the command line; they are not picked up from the diff.

Configuration
-------------
The configuration file allows individual or groups of checks to enabled or
disabled, and specifies the enterprise namespace in which components live. By
default, that namespace is ``BloombergLP``, and almost all checks are enabled.
The configuration file consists of a set of options, one per line, processed
in order. Additional configuration lines may be supplied on the command line
as described above. In particular, specifying ``-cl 'load file'`` will
augment the default configuration with the contents of ``file``.

Checks and Tags
---------------
|Bv| implements a set of *checks*\ , each representing a category of issues to
be detected.  Each such check may result in one or more types of warning being
issued, and those warnings are prefixed with a *tag* consisting of capital
letters followed by digits.  If a check is disabled, none of its warnings will
appear.  If a check is enabled, individual tags may optionally be suppressed.
Each check and tag is described later in this document.  The configuration file 
is used to enable or disable individual checks and tags.

=============================== ===============================================
Config Entry                    Description
=============================== ===============================================
``#`` *text*                    Comment text.
``namespace`` *name*            Enterprise namespace.
``all on``                      Turn all checks on.
``all off``                     Turn all checks off.
``group`` *groupname* *name*... Make *groupname* a synonym for the list of
                                *name*\ s (which may themselves be group
                                names).
``check`` *name* ``on``         Turn specific check or group on.
``check`` *name* ``off``        Turn specific check or group off.
``load`` *file*                 Read and process configuration lines from the
                                *file*.
``set`` *parameter value*       Set a parameter used by a check.
``append`` *parameter value*    Append to a parameter used by a check.
``prepend`` *parameter value*   Prepend to a parameter used by a check.
``suppress`` *tag* *files*...   Messages with the specified *tag* are
                                suppressed for the specified *files*. Either
                                *tag* or *files* (but not both) may be ``*``.
                                The *tag* may be a group *name*, suppressing
                                all members (including subgroups).
``unsuppress`` *tag* *files*... Messages with the specified *tag* are
                                unsuppressed for the specified *files*. Either
                                *tag* or *files* (but not both) may be ``*``.
                                The *tag* may be a group *name*, unsuppressing
                                all members (including subgroups).  Note that
                                only previously suppressed tag/file pairs can
                                be unsuppressed.
=============================== ===============================================

If the configuration file attempts to name a non-existent check, the tool will
report a list of all known checks and then exit. Do this deliberately to obtain
an accurate list of checks if you suspect this documentation is out of date.

Local Suppressions
------------------

The |bv| command can locally suppress or enable individual message tags within
a source file region, using ``#pragma`` |bv| constructs or ``//`` |BV|
``pragma:`` structured comments.

Note that programs are often compiled with options that generate warnings for
unknown pragmas; |bv| defines the macro |BV| to enable enclosing these pragmas
within ``#ifdef`` |BV| blocks.

Local suppressions operate within a single file, and will not have any effect
on warnings in files that this file includes or in files that include this one.

Note that this cannot enable a check which was disabled by ``check name off``
in the configuration.

+------------------------------+----------------------------------------------+
| Pragma                       | Effect                                       |
+==============================+==============================================+
| ``#pragma`` |bv| ``-TAG``    | From this point forward in the file, do not  |
+------------------------------+ report *TAG* messages. *TAG* may be a group  |
| ``//`` |BV| ``pragma: -TAG`` | *name*.                                      |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv| ``+TAG``    | From this point forward in the file, report  |
+------------------------------+ *TAG* messages. *TAG* may be a group *name*. |
| ``//`` |BV| ``pragma: +TAG`` |                                              |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv| ``push``    | Save the suppressions and parameters state   |
+------------------------------+ of the current file.                         |
| ``//`` |BV| ``pragma: push`` |                                              |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv| ``pop``     | Restore the suppressions and parameters      |
+------------------------------+ state of the current file as of the most     |
| ``//`` |BV| ``pragma: pop``  | recent active ``push``.                      |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv|             | Set the configuration *parameter* to         |
| ``set parameter value``      | *value*.                                     |
+------------------------------+                                              |
| ``//`` |BV| ``pragma:``      |                                              |
| ``set parameter value``      |                                              |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv|             | Append *value* to the configuration          |
| ``append parameter value``   | *parameter*.                                 |
+------------------------------+                                              |
| ``//`` |BV| ``pragma:``      |                                              |
| ``append parameter value``   |                                              |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv|             | Prepend *value* to the configuration         |
| ``prepend parameter value``  | *parameter*.                                 |
+------------------------------+                                              |
| ``//`` |BV| ``pragma:``      |                                              |
| ``prepend parameter value``  |                                              |
+------------------------------+----------------------------------------------+
| ``#pragma`` |bv|             | For purposes of transitive inclusion         |
| ``re-export <file>``         | detection, indicate that inclusion of the    |
+------------------------------+ containing file satisfies the need to        |
| ``//`` |BV| ``pragma:``      | include *file*.                              |
| ``re-export <file>``         |                                              |
+------------------------------+----------------------------------------------+

Exit Status
-----------

Normally, the exit status of a |bv| run is 0 (success) unless the code has
actual errors.  If a particular check or tag is produced and that check or tag
is set in the *failstatus* configuration parameter, the exit status will be 1
(failure).  This allows for the creation of wrapper scripts whose exit status
indicates that some condition fails to hold.

Checks
------

These are the checks supported by the tool.  (A few are of dubious value and
may be removed in the future.)  We welcome suggestions for additional checks.

.. only:: bde_verify or bb_cppverify

   allocator-forward
   +++++++++++++++++

   Checks dealing with allocator forwarding and traits.  Allocator-aware
   classes have a number of requirements, such as having constructors that
   accept allocator parameters, passing those parameters to constructors
   of sub-objects, and setting type traits correctly.

   An experimental and preliminary feature has been added to this check to
   enable automatic allocatorization of classes via the rewriting facility.
   Name the classes to be transformed in the configuration file parameter
   ``allocator_transform``.  Use the ``-rewrite`` option to generate the
   rewritten file.

   * ``AT01``
     Class does not use allocators but has an affirmative allocator trait.
   * ``AT02``
     Class uses allocators but has no affirmative or negative allocator trait.
   * ``AC01``
     A class which uses allocators has a constructor with no variant that can
     be called with an allocator.
   * ``AC02``
     A class which uses allocators has an implicit copy constructor that cannot
     be called with an allocator.
   * ``MA01``
     A constructor of a class that uses allocators and takes an allocator does
     not pass the allocator to constructors of base classes that take
     allocators.
   * ``MA02``
     A constructor of a class that uses allocators and takes an allocator does
     not pass the allocator to constructors of class members that take
     allocators.
   * ``AM01``
     An explicit allocator argument to a constructor expression initializes a
     non-allocator parameter of that constructor.
   * ``AR01``
     An object of a type with an affirmative allocator trait is returned by
     value.
   * ``GA01``
     A variable with global storage must be initialized with a non-default
     allocator.
   * ``BT01``
     A class trait declaration does not mention its class name.
   * ``RV01``
     Function should return by value rather than through pointer parameter.
   * ``AU01``
     An allocator argument needs to be manually checked for appropriateness.
     This is intended to catch assignment idioms like
     ``MyClass(other, this->allocator()).swap(*this)`` that can exhaust
     sequential allocators (but are sometimes necessary).
   * ``AP01``
     A class has an unnecessary ``d_allocator_p`` pointer.  (The allocator can
     be retrieved from a subobject.)
   * ``AP02``
     A class is lacking a necessary ``d_allocator_p`` pointer.  (The class
     uses allocators and has no allocator-aware subobjects.)
   * ``AL01``
     A class is lacking a necessary ``allocator()`` method.  (The class uses
     allocators and should offer a method to retrieve the one used.)
   * ``AH01``
     Messages relating to the generation of assignment operators as part of
     automatic allocatorization.
   * ``WT01``
     Automatic allocatorization cannot be performed for classes with array
     members.

.. only:: bde_verify or bb_cppverify

   allocator-new
   +++++++++++++

   In BDE code, a placement new overload is provided that takes an allocator
   reference.  Passing an allocator pointer to placement new will not call
   that overload.

   * ``ANP01``
     Calls to placement new with an argument that is a pointer to an allocator.

.. only:: bde_verify

   alphabetical-functions
   ++++++++++++++++++++++

   BDE coding guidelines specify that functions in a group should be in
   alphanumeric order.

   * ``FABC01``
     Functions in a component section that are not in alphanumeric order.

   Note that the ordering resets in certain cases, such as when a pair of
   functions are not from the same context.

   Ordering also resets across single-line comments such as
   ``// CLASS METHODS`` and line banners.

.. only:: bde_verify or bb_cppverify

   Header files should not contain anonymous namespaces, because each
   compilation unit that includes such a header gets a separate instance
   of that namespace, and that is generally not wanted.

   anon-namespace
   ++++++++++++++
   * ``ANS01``
     Anonymous namespace in header.

.. only:: bde_verify or bb_cppverify

   array-argument
   ++++++++++++++

   A function parameter that is declared as an array with a specified size
   is really just a pointer, and having the size present is misleading.

   * ``AA01``
     Sized array parameter is really a pointer.

.. only:: bde_verify

   array-initialization
   ++++++++++++++++++++

   Warn when an array initializer that has fewer elements than the array
   size has a final initializer that is not the default element value, to
   guard against incorrect initialization.

   * ``II01``
     Incomplete array initialization in which the last value is not the default
     member value.

.. only:: bde_verify or bb_cppverify

   assert-assign
   +++++++++++++

   Assertion conditions are often a top-level "expected == actual" expression
   and may erroneously be written as an "expected = actual" assignment.

   * ``AE01``
     Top-level macro condition is an assignment.

.. only:: bde_verify

   banner
   ++++++

   BDE coding guidelines have a variety of banner requirements, for example::

                            // ==============
                            // class abcd_efg
                            // ==============

   for class definitions.  This check catches a few style violations.

   * ``BAN02``
     Banner rule lines do not extend to column 79.
   * ``BAN03``
     Banner text is not centered properly within configuration file parameter
     ``banner_slack`` spaces left or right (default 5).
   * ``BAN04``
     Banner text underlining is not centered properly.
   * ``FB01``
     Inline functions in header require ``// INLINE DEFINITIONS`` banner.

.. only:: bde_verify

   base
   ++++

   |Bv| detects pragmas and comments that direct it to save and restore its
   internal state using a stack, and checks that the stack is manipulated
   appropriately.

   * ``PR01``
     ``#pragma`` |bv| ``pop`` when stack is empty.
   * ``PR02``
     ``#pragma`` |bv| ``push`` is never popped.

.. only:: bde_verify

   boolcomparison
   ++++++++++++++

   Rather than comparing boolean values against 'true' or 'false', they
   should be tested directly, i.e., ``if (!cond)`` rather than
   ``if (false == cond)``.

   * ``BC01``
     Comparison of a Boolean expression with literal ``true`` or ``false``.

.. only:: bde_verify

   bsl-overrides-std
   +++++++++++++++++

   Rewrite code which compiles with ``BSL_OVERRIDES_STD`` defined to not
   require that.  Use the ``-rewrite`` option to generate the rewritten file.

   Note that ``BSL_OVERRIDES_STD`` is now obsolete and Bloomberg internal code
   has already been changed not to use it.

   * ``IS01``
     Include of header is needed to declare a symbol.
   * ``IS02``
     Inserting include of header.
   * ``SB01``
     Replacing one header with another.
   * ``SB02``
     Replacing one include guard with another.
   * ``SB03``
     Removing include guard definition.
   * ``SB04``
     Replacing use of macro ``std`` with ``bsl``.
   * ``SB07``
     Replacing ``std`` with ``bsl`` in macro definition.

.. only:: bde_verify or bb_cppverify

   bsl-std-string
   ++++++++++++++

   This check warns that conversions between bsl::string and std::string
   are occurring (in case they are inadvertant).

   * ``ST01``
     Converting std::string to bsl::string.
   * ``ST02``
     Converting bsl::string to std::string.

.. only:: bde_verify or bb_cppverify

   c-cast
   ++++++

   Discourage use of C-style cast expressions.

   * ``CC01``
     C-style cast expression. (Dispensation is granted to ``(void)expr``.)

.. only:: bde_verify or bb_cppverify

   char-classification-range
   +++++++++++++++++++++++++

   Detect that signed character or too-large arguments are being passed to
   standard library character classification functions.  Those functions
   require that their parameters lie in the range [-1 .. 255].

   * ``ISC01``
     ``char`` variable passed to ``is...`` function may sign-extend, causing
     undefined behavior.
   * ``ISC02``
     ``char`` constant passed to ``is...`` function may sign-extend, causing
     undefined behavior.
   * ``ISC03``
     Out-of-range value passed to ``is...`` function may cause undefined
     behavior.

.. only:: bde_verify or bb_cppverify

   char-vs-string
   ++++++++++++++

   A ``const char *`` function parameter is usually expected to be the
   address of a null-terminated character array, and passing the address
   of a single character as an argument may be a program-logic error.

   * ``ADC01``
     Passing the address of a single character as an argument to a
     ``const char *`` parameter.

.. only:: bde_verify

   class-sections
   ++++++++++++++

   BDE coding standards require that class member declarations appear in tagged
   sections (e.g., ``// MANIPULATORS``, ``// CREATORS``, ``// PUBLIC DATA``, et
   al.)  This check verifies that tags are present for declarations at all, and
   if so, that they match the accessibility and types of the declarations.

   * ``KS00``
     Declaration not preceed by section tag comment.
   * ``KS01``
     Tag requires public declaration.
   * ``KS02``
     Tag requires private declaration.
   * ``KS03``
     Tag requires function declaration.
   * ``KS04``
     Tag requires instance data field declaration.
   * ``KS05``
     Tag requires static data field declaration.
   * ``KS06``
     Tag requires type declaration.
   * ``KS07``
     Tag requires const method declaration.
   * ``KS08``
     Tag requires non-const method declaration.
   * ``KS09``
     Constructor or destructor requires CREATORS tag.
   * ``KS10``
     Tag requires constant data declaration.
   * ``KS11``
     Tag requires static method declaration.
   * ``KS12``
     Tag requires free operator declaration.
   * ``KS13``
     Tag requires free function declaration.
   * ``KS14``
     Tag requires conversion operator declaration.
   * ``KS15``
     Friend declaration requires FRIENDS tag.
   * ``KS16``
     Tag requires friend declaration.
   * ``KS17``
     Tag requires protected declaration.

.. only:: bde_verify

   comments
   ++++++++

   Comments containing erroneous or deprecated text according to BDE coding
   standards or general lore.

   * ``FVS01``
     Deprecate the phrase *fully value semantic*.
   * ``BADB01``
     Single-line inheritance bubbles in comments.
   * ``AD01``
     Bubble display picture should begin in column 5.
   * ``BW01``
     Comment text could fit at end of previous comment line, leaving it less
     than 80 - parameter ``wrap_slack`` (default 1) characters long.
   * ``PRP01``
     ``//@PURPOSE:`` line is recognizable but malformed.
   * ``PP01``
     Deprecate the phrase *pure procedure*.
   * ``DC01``
     ``//@DESCRIPTION:`` should contain single-quoted class name.
   * ``CLS01``
     ``//@CLASSES:`` should not contain class names on that line.
   * ``CLS02``
     ``//@CLASSES:`` classes should be followed by colon and description.
   * ``CLS03``
     Badly formatted class line.
   * ``MOR01``
     Deprecate the phrase *(non-)modifiable reference*.
   * ``PSS01``
     Use two spaces after a period.

.. only:: bde_verify

   comparison-order
   ++++++++++++++++

   In order to guard against accidental assignment (``=`` when ``==`` was
   meant), equality comparisons between constant and non-constant expressions
   should have the constant expression on the left.

   * ``CO01``
     Non-modifiable operand should be on the left.
   * ``CO02``
     Constant-expression operand should be on the left.

.. only:: bde_verify or bb_cppverify

   component-header
   ++++++++++++++++

   A component implementation file should include the component header file,
   and the component header should be the first included header.

   * ``TR09``
     Component implementation file does not include its header file ahead of
     other includes or declarations.

.. only:: bde_verify or bb_cppverify

   component-prefix
   ++++++++++++++++

   BDE coding style requires that globally visible names provided by a
   component have the component name as a prefix.  For example, the BDE
   component bdlt_calendar provides ``bdlt::Calendar_BusinessDayConstIter`` as
   well as ``bdlt::Calendar`` itself.  This rule applies to macros as well.

   * ``CP01``
     Globally visible name is not prefixed by component name.

   Will not warn about packages included in parameter ``global_packages``
   (default ``bslmf bslstl``).

.. only:: bde_verify

   constant-return
   +++++++++++++++

   Discourage the use of functions that just return a constant value.

   * ``CR01``
     Single statement function returns a constant value.

.. only:: bde_verify

   contiguous-switch
   +++++++++++++++++

   Switch statements in ``main`` with case labels that do not match
   BDE-standard test-driver order (0 with no ``break;`` then contiguous values
   in descending order each with a ``break;``, then ``default``).

   * ``ES01``
     Empty ``switch`` statement.
   * ``SD01``
     The first case is ``default``.
   * ``SZ01``
     The first case is not ``0``.
   * ``MD01``
     The ``default`` case is not last.
   * ``LO01``
     Case labels are out of order.
   * ``ED01``
     No ``default`` case at end of ``switch``.
   * ``CS01``
     Test case code is not inside braces.
   * ``CS02``
     Test case code is not inside single set of braces.
   * ``MB01``
     Missing ``break`` before ``case``.
   * ``ZF02``
     ``case 0`` does not just fall through to next case.
   * ``SM01``
     Missing cases in switch.

.. only:: bde_verify or bb_cppverify

   cpp-in-extern-c
   +++++++++++++++

   Header files with C++ constructs included within ``extern "C"`` contexts.

   * ``PC01``
     C++ header included within C linkage specification.

.. only:: bde_verify or bb_cppverify

   deprecated
   ++++++++++

   Detect use of deprecated functions and types.

   * ``DP01``
     Call to deprecated function.

.. only:: bde_verify or bb_cppverify

   do-not-use-endl
   +++++++++++++++

   Discourage use of ``endl`` because it flushes the output stream and can
   therefore cause programs to be unnecessarily slow.  Rather output ``\\n``
   and use ``flush`` in the rare times it's explicitly needed.

   * ``NE01``
     Prefer using ``'\\n'`` over ``endl``.

.. only:: bde_verify or bb_cppverify

   entity-restrictions
   +++++++++++++++++++

   BDE style recommends having names be declared within component classes,
   not at global scope.

   * ``TR17``
     Items declared in global scope.

.. only:: bde_verify

   enum-value
   ++++++++++

   BDE guidelines call for using ``Enum`` as the name of an enumeration type
   within a component.  The previously commonly used ``Value`` is obsolete.

   * ``EV01``
     Component enumeration tag is ``Value``.

.. only:: bde_verify or bb_cppverify

   external-guards
   +++++++++++++++

   Header files should be guarded against multiple inclusion, like so::

       // abcd_efg.h
       #ifndef INCLUDED_ABCD_EFG
       #define INCLUDED_ABCD_EFG
           // ... stuff ...
       #endif

   Formerly, BDE style required guard checking in headers, as in the following
   code, but this is now obsolete.  The ``SEG03`` warning is that this check is
   missing and the ``SEG04`` warning is that this check is present.  (The
   former is disabled in the default configuration file.)::

       // abcd_xyz.h
       #ifndef INCLUDED_ABCD_EFG
       #include <abcd_efg.h>
       #endif

   * ``SEG01``
     Include guard without include file.
   * ``SEG02``
     Include guard does not match include file.
   * ``SEG03``
     File included in header without include guard test.
   * ``SEG04``
     File included in header with include guard test.

.. only:: bde_verify

   files
   +++++

   Missing or inaccessible component header file or test driver.  BDE style
   requires that a component have a header file, an implementation file, and a
   test driver file.

   * ``FI01``
     Component header file is missing.
   * ``FI02``
     Component test driver file is missing.

.. only:: bde_verify or bb_cppverify

   free-functions-depend
   +++++++++++++++++++++

   Free functions (not part of a component class) declared in a header file
   should have a parameter whose type is declared by that header file.

   * ``AQS01``
     Free function parameter must depend on a local definition.

.. only:: bde_verify or bb_cppverify

   friends-in-headers
   ++++++++++++++++++

   BDE style requires that if a class or method is granted friendship, that
   entity must be declared in the same header file.  (We call this the dictum
   of "no long-distance friendship").

   * ``AQP01``
     Friends must be declared in the same header.

.. only:: bde_verify

   function-contract
   +++++++++++++++++
   
   Incorrect or missing function contracts.  BDE coding guidelines describe the
   detailed requirements, including indentation, position, and the proper way
   of documenting parameters.  A correct example is::

       double total(double amount, int number = 1);
           // Return the total amount to charge for an order where one item
           // costs the specified 'amount'.  Optionally specify the 'number'
           // of items in the order.  If 'number' is not specified, a single
           // item is assumed.

   * ``FD01``
     Missing contract.
   * ``FD02``
     Contract indented incorrectly.
   * ``FD03``
     Parameter is not documented.
   * ``FD04``
     Parameter name is not single-quoted.
   * ``FD05``
     Parameters with default values are not called out with *optionally
     specify*.
   * ``FD06``
     Parameters are not called out with *specified*.
   * ``FD07``
     Parameter called out with *specified* more than once.

.. only:: bde_verify or bb_cppverify

   global-data
   +++++++++++

   Programs should not contain global data outside of classes.

   * ``AQb01``
     Data variable with global visibilty.

.. only:: bde_verify or bb_cppverify

   global-function-only-in-source
   ++++++++++++++++++++++++++++++

   Globally visible functions must be declared in header files.

   * ``TR10``
     Globally visible function not declared in header.

.. only:: bde_verify or bb_cppverify

   global-type-only-in-source
   ++++++++++++++++++++++++++

   Globally visible types must be declared in header files.

   * ``TR10``
     Globally visible type not declared in header.
   * ``TR11``
     Globally visible type should be defined in header.

.. only:: bde_verify

   groupname
   +++++++++

   BDE style requires a particular layout for component file locations - for
   example, the component header abcd_efg.h is expected to be found as
   ``abc/abcd/abcd_efg.h``.

   * ``GN01``
     Component does not have a distinguishable correctly formed package group
     name.
   * ``GN02``
     Component is not located within its correct package group directory.

.. only:: bde_verify or bb_cppverify

   hash-pointer
   ++++++++++++

   When a pointer is passed to a call of an object of type std::hash<TYPE*>,
   the hash will apply to the value of the pointer rather than to what the
   pointer points.  This is generally not what is wanted.

   * ``HC01``
     Warn that use of ``std::hash<TYPE*>()(ptr)`` uses only the value and not
     the contents of *ptr*.

.. only:: bde_verify

   headline
   ++++++++

   The first line of a component file should start with ``// file_name`` and
   end in column 79 with with ``-*-C++-*-``.

   * ``HL01``
     The headline of the file is incorrect.

.. only:: bde_verify

   implicit-ctor
   +++++++++++++

   Constructors that are not designated ``explicit`` and take one argument can
   be used to implicitly convert that argument to class type.  They should be
   tagged with an ``// IMPLICT`` comment.

   * ``IC01``
     Non-``explicit`` constructor which may be invoked implicitly and
     not marked with ``// IMPLICIT``

.. only:: bde_verify or bb_cppverify

   in-enterprise-namespace
   +++++++++++++++++++++++

   All top-level declarations should be within the enterprise namespace.

   * ``AQQ01``
     Declaration not in enterprise namespace.

.. only:: bde_verify or bb_cppverify

   include-guard
   +++++++++++++

   Header files should be protected against multiple inclusion using guards::

       // abcd_efg.h
       #ifndef INCLUDED_ABCD_EFG
       #define INCLUDED_ABCD_EFG
           // ... stuff ...
       #endif

   The include guard is expected to properly match its file name and be used as
   above.

   * ``TR14``
     Header file does not set up or use its include guard macro properly.

.. only:: bde_verify

   include-in-extern-c
   +++++++++++++++++++

   Header files should not be included inside ``extern "C" { }`` sections
   because being declared within "C" linkage can change the meaning of the
   constructs they contain.

   * ``IEC01``
     Header file included within C linkage specification.

.. only:: bde_verify

   include-order
   +++++++++++++

   Header files are not included in BDE-standard order.

   * ``SHO01``
     Headers out of order.
   * ``SHO02``
     Header comes too late in order.
   * ``SHO03``
     Component does not include its header.
   * ``SHO04``
     Component does not include its header first.
   * ``SHO06``
     ``_...__ident.h`` file not included.
   * ``SHO07``
     ``_..._scm_version.h`` file not included.
   * ``SHO08``
     Header and source use ``bdes_ident.h`` inconsistently.
   * ``SHO09``
     ``bsls`` components should not include ``_...__ident.h``.

.. only:: bde_verify

   indentation
   +++++++++++

   BDE coding standards have a variety of indentation formatting requirements.

   * ``IND01``
     Line is (possibly) mis-indented.
   * ``IND02``
     Function parameters should be all or each on one line.
   * ``IND03``
     Function parameters on multiple lines should align vertically.
   * ``IND04``
     Declarators on multiple lines should align vertically.
   * ``IND05``
     Template parameters should be all or each on one line.
   * ``IND06``
     Template parameters on multiple lines should align vertically.

   Indentation checking is currently disabled in the default configuration file
   until more experience is gained, to avoid cascades of warnings.

   Code between ``//..`` display elements is not checked.

.. only:: bde_verify

   leaking-macro
   +++++++++++++

   Macros that are left defined at the end of a header file must begin with
   the name of the component (unless they are include guard macros, which have
   their own form).

   * ``SLM01``
     Component header file macro neither an include guard nor prefixed by
     component name.

.. only:: bde_verify or bb_cppverify

   local-friendship-only
   +++++++++++++++++++++

   "Long-distance" friendship is not permitted.

   * ``TR19``
     Friendship granted outside of component.

.. only:: bde_verify

   long-inline
   +++++++++++

   Very long functions in header files (often function templates) should not be
   declared inline if they are too long.  "Too long" is defined by the
   configuration variable ``max_inline_lines``.  (|Bv| will count statements
   rather than physical lines.)

   * ``LI01``
     Inline function is longer than configuration file parameter
     ``max_inline_lines`` (default 10).

.. only:: bde_verify

   longlines
   +++++++++

   BDE style requires that lines be no longer than 79 characters long.
   By request of the |bv| management, this is not a configurable value.

   * ``LL01``
     Line exceeds 79 characters.

.. only:: bde_verify or bb_cppverify

   managed-pointer
   +++++++++++++++

   Probabale or possible inconsistent uses of allocators and deleters when
   icreating 'ManagedPtr' or 'shared_pointer'.  The warnings below are also
   accompanied by notes saying to consider using 'allocateManaged' or
   'allocate_shared', which prevent these problems.  The 'MPOK01' warning is
   typically disabled, representing usages that are likely to be correct even
   though they are not expressed in the preferred way.

   * ``MPOK01``
     Shared pointer without deleter using default-assigned allocator variable.

     Shared pointer without deleter using default-initialized allocator
     variable.

     Shared pointer without deleter using default allocator directly.

     Shared pointer should use allocator member as deleter.
   * ``MP01``
     Shared pointer without deleter will use 'operator delete'.
   * ``MP02``
     Different allocator and deleter for shared pointer.
   * ``MP03``
     Deleter provided for non-placement allocation for shared pointer.

.. only:: bde_verify

   member-definition-in-class-definition
   +++++++++++++++++++++++++++++++++++++

   BDE style requires that methods be declared in classes but defined outside
   of them.

   * ``CD01``
     Method defined directly in class definition.

.. only:: bde_verify or bb_cppverify

   member-names
   ++++++++++++

   BDE style requires that data members of classes (but not ``structs``) be
   private, and that paerticular naming conventions be followed.

   * ``MN01``
     Class data members must be private.
   * ``MN02``
     Non-static class data member names must start with ``d_``.
   * ``MN03``
     Static class data member names must start with ``s_``.
   * ``MN04``
     Pointer class data member names must end in ``_p``.
   * ``MN05``
     Only pointer class data member names should end in ``_p``.

.. only:: bde_verify

   move-contract
   +++++++++++++

   Uses the rewriting facility to move function contracts above functions (and
   shift them four spaces left).  Note that this feature is preliminary, and
   other checks that require contracts do not look for them in this position.
   Use the ``-rewrite`` option to generate the rewritten file.

   * ``CM01``
     Contract being moved above function.

.. only:: bde_verify

   mid-return
   ++++++++++

   BDE style requires that ``return`` statements in functions, other than the
   final one, be tagged with a ``// RETURN`` comment ending in column 79.

   * ``MR01``
     Non-final ``return`` statement not tagged with ``// RETURN``.
   * ``MR02``
     ``// RETURN`` tag does not end in column 79.

.. only:: bde_verify

   namespace-tags
   ++++++++++++++

   The closing brace of a multi-line namespace declaration should be marked
   with one of these comments::

       }  // close enterprise namespace
       }  // close package namespace
       }  // close unnamed namespace
       }  // close namespace name

   * ``NT01``
     Multi-line namespace blocks must end with
     ``// close [ enterprise | package | unnamed | description ] namespace``.

.. only:: bde_verify

   nested-declarations
   +++++++++++++++++++

   Declarations should be nested within a package namespace inside the
   enterprise namespace.

   * ``TR04``
     Declarations not properly nested in package namespace.

     Will not warn about main files unless parameter ``main_namespace_check``
     is ``on`` (default ``off``).

     Will not warn about packages included in parameter ``global_packages``
     (default ``bslmf bslstl``).

.. only:: bde_verify or bb_cppverify

   nonascii
   ++++++++

   Source code should contain only 7-bit ASCII characters.

   * ``NA01``
     Source code contains bytes with value greater than 127.

.. only:: bde_verify

   operator-void-star
   ++++++++++++++++++

   Classes should not contain operators that permit them to be implictly
   converted to ``void *`` or ``bool`` to prevent accidental misuse.

   * ``CB01``
     Class contains conversion operator to ``void *`` or ``bool``.

.. only:: bde_verify

   packagename
   +++++++++++

   Component package name or location does not follow BDE convention.

   * ``PN01``
     Only one underscore in standalone component file name.
   * ``PN02``
     Component part of filename should be prefixed by package name.
   * ``PN03``
     Package part of name should be group name followed by 1-4 characters.
   * ``PN04``
     Package and group names must be lower-case and not start with a digit.
   * ``PN05``
     Component is not located within its correct package directory.

.. only:: bde_verify or bb_cppverify

   ref-to-movableref
   +++++++++++++++++

   BDE provides a ``MovableRef`` type meant to simulate rvalue references in
   C++03 code.  Objects of this type should be passed by value.

   * ``MRR01``
     MovableRef should be passed by value, not reference.

.. only:: bde_verify

   refactor
   ++++++++

   Uses the rewriting facility to change included files and use of names.
   Specification is done via the parameter ``refactor`` in the configuration
   file.  Use the ``-rewrite`` option to generate the rewritten file.
   
   To replace an included file, specify ``file(old[,new]*)``; the include of
   the old header file will be removed and replaced by the new ones, if any. If
   the old header was surrounded by redundant include guards, the replacements
   will be as well.  E.g., ``append refactor file(bdet_date.h,bdlt_date.h)``.

   To replace a name, specify ``name(old,new)``; the old name should be fully
   elaborated with namespaces and classes, except for the enterprise namespace
   (``BloombergLP``).  Appearances of the old name, elaborated or not, will be
   replaced by the specified new value.  E.g.,
   ``append refactor name(bdetu_DayOfWeek::Day::BDET_WEDNESDAY,e_WEDNESDAY)``.
   Macro names may also be replaced this way; just specify the old and the new.

   * ``RX01``
     Errors in the refactor specification (not in the examined files).
   * ``RF01``
     Replacing included files.
   * ``RC01``
     Replacing a name.
   * ``RD01``
     Replacing forward class declaration.

.. only:: bde_verify

   refactor-config
   +++++++++++++++

   Given pairs of old/new header files, generate a configuration file for the
   ``refactor`` check from corresponding pairs of names appending to the file
   specified by the configuration file parameter ``refactorfile`` (or the
   default, "refactor.cfg" if left unspecified).

   * ``DD01``
     Eligible name for refactoring.

.. only:: bde_verify or bb_cppverify

   runtime-initialization
   ++++++++++++++++++++++

   Global variables with initializers that run when the program is loaded are
   error-prone, although less so when they appear in the prgram file containing
   ``main()``.

   * ``AQa01``
     Global variable with runtime initialization in file without main().
   * ``AQa02``
     Global variable with runtime initialization in file with main().

.. only:: bde_verify or bb_cppverify

   short-compare
   +++++++++++++

   * ``US01``
     Comparison between signed and unsigned short may cause unexpected
     behavior.  Signed and unsigned shorts in expressions are both promoted
     to integer, with sign-extension for signed short and zero-extension
     for unsigned short.  Thus a signed short and an unsigned short with the
     same bit values and the high bit set will convert to different integer
     values.

.. only:: bde_verify

   spell-check
   +++++++++++

   Spell-checking is disabled by default in the config file
   (``check spell-check off``) to avoid noise.

   Words in configuration parameter ``dictionary`` are assumed correct.
   Extra words can be added to a config file or when the program is run::

       |bv| -cl 'append dictionary treeify unsquash' ...

   Words that appear at least as many times as non-zero configuration
   parameter ``spelled_ok_count`` (default 3) are assumed correct.

   The spell checker is the library version of `GNU Aspell`_.

   .. _GNU Aspell: http://aspell.net

   * ``SP01``
     Misspelled word in comment.
   * ``SP02``
     Cannot start spell checker.  (Not an error in the examined file.)
   * ``SP03``
     Misspelled word in parameter name.


.. only:: bde_verify or bb_cppverify

   strict-alias
   ++++++++++++

   C++ grows ever less fond of type punning.  Casting between pointer types
   (except for void and char types) will trigger this check.

   * ``SAL01``
     Possible strict-aliasing violation.

.. only:: bde_verify or bb_cppverify

   string-add
   ++++++++++

   Adding an integer to a string literal is deemed suspect.

   * ``SA01``
     Addition of integer and string literal.

.. only:: bde_verify

   swap-a-b
   ++++++++

   BDE style requires that the parameters of a free ``swap`` functionbe named
   ``a`` and ``b``.

   * ``SWAB01``
     Parameters of free *swap* function are not named *a* and *b*.

.. only:: bde_verify or bb_cppverify

   swap-using
   ++++++++++

   Directly invoking ``std::swap`` or ``bsl::swap`` can prevent argument-
   dependent lookup from finding overloads.

   * ``SU01``
     Prefer ``using std::swap; swap(...);'`` over ``std::swap(...);``.

.. only:: bde_verify

   template-typename
   +++++++++++++++++

   BDE coding style requires that template type parameters be designated with
   ``class`` rather than ``typename``, that they not be single-letter names,
   and that they should be in all-capital letters.

   * ``TY01``
     Use of ``typename`` instead of ``class`` in ``template`` header.
   * ``TY02``
     Use of single-letter template parameter names.
   * ``TY03``
     Use of non ``ALL_CAPS`` template parameter names.

.. only:: bde_verify

   test-driver
   +++++++++++

   Checks for test drivers.

   * ``TP02``
     TEST PLAN section is missing ``//-...-`` separator line.
   * ``TP03``
     TEST PLAN item is missing a test number.
   * ``TP04``
     TEST PLAN item test number is zero.
   * ``TP05``
     Test case without comment.
   * ``TP06``
     Test case does not list item from TEST PLAN.
   * ``TP07``
     TEST PLAN item is empty.
   * ``TP08``
     Item is mentioned in test case comment but that number is not in TEST PLAN
     item.
   * ``TP09``
     Item is mentioned in test case comment but not in TEST PLAN.
   * ``TP10``
     Test driver ``case 0:`` has a test comment.
   * ``TP11``
     Test driver has no ``switch`` statement in ``main()``.
   * ``TP12``
     Test case comment has no ``// Testing:`` line.
   * ``TP13``
     TEST PLAN has no items.
   * ``TP14``
     Test driver has no TEST PLAN.
   * ``TP15``
     ``// Testing:`` line in test comment is recognizable but not exactly
     correct.
   * ``TP16``
     Extra characters in TEST PLAN items before ``[ ]``.
   * ``TP17``
     Test case does not start with ``if (verbose)`` print banner...
   * ``TP18``
     Test case printed banner is formatted incorrectly.
   * ``TP19``
     Test driver has various missing or malformed boilerplate sections.
   * ``TP20``
     Within loop in test case, action under ``if (verbose)`` rather than a
     very verbose flag.
   * ``TP21``
     Within loop in test case, no action under a (very) verbose flag.
   * ``TP22``
     Test case title does not match printed banner.
   * ``TP23``
     ``main()`` should end with ``return testStatus;``.
   * ``TP24``
     ``default`` case should set ``testStatus = -1;``.
   * ``TP25``
     Cannot find definition of class mentioned in ``//@CLASSES:``.
   * ``TP26``
     Test plan does not cover all public functions of a class mentioned in
     ``//@CLASSES:``.
   * ``TP27``
     Public function of a class in ``//@CLASSES:`` is not called from the test
     driver.
   * ``TP28``
     Test case has mis-formatted ``// Concerns:`` line.
   * ``TP29``
     Test case has improperly numbered concern.
   * ``TP30``
     Test case is missing ``Concerns:`` section.
   * ``TP31``
     Test case has mis-formatted ``// Plan:`` line.
   * ``TP32``
     Test case has improperly numbered plan.
   * ``TP33``
     Test case is missing ``Plan:`` section.

.. only:: bde_verify

   that-which
   ++++++++++

   Grammar check preferring the word ``that`` to ``which`` in many cases.

   * ``TW01``
     Prefer ``that`` to ``which``.
   * ``TW02``
     Possibly incorrect comma before ``that``.

.. only:: bde_verify or bb_cppverify

   throw-non-std-exception
   +++++++++++++++++++++++

   Thrown exception objects should inherit from ``std::exception``.

   * ``FE01``
     Throwing exception not derived from ``std::exception``.

.. only:: bde_verify or bb_cppverify

   transitive-includes
   +++++++++++++++++++

   A source files should include all headers that declare names used by that
   source file, even when those headers would be included indirectly.

   * ``AQK01``
     Header included transitively should be included directly.
   * ``AQK02``
     ``<bsls_buildtarget.h>`` needed for ``BDE_BUILD_TARGET_...`` macros.

.. only:: bde_verify or bb_cppverify

   unnamed-temporary
   +++++++++++++++++

   A temporary unnamed object will be immediately destroyed, and it is
   unlikely to be the intended use.  The canonical example of this error
   is ``mutex m; mutex_guard(&m);``.

   * ``UT01``
     Unnamed object will be immediately destroyed.

.. only:: bde_verify

   upper-case-names
   ++++++++++++++++

   BDE style does not permit variable and type names to be all upper-case.

   * ``UC01``
     Names of variables and types should not be all upper-case.

.. only:: bde_verify or bb_cppverify

   using-declaration-in-header
   +++++++++++++++++++++++++++

   Header files should not contain ``using`` declarations because they can
   cause mysterious name clashes in files that include them.

   * ``TR16``
     Header file contains ``using`` declaration.
   * ``AQJ01``
     Using declaration precedes header inclusion.

   Will not warn about packages included in parameter ``global_packages``
   (default ``bslmf bslstl``).

.. only:: bde_verify or bb_cppverify

   using-directive-in-header
   +++++++++++++++++++++++++

   Header files should not contain ``using`` directives because they can
   cause mysterious name clashes in files that include them.

   * ``TR16``
     Header file contains ``using`` directive.
   * ``AQJ02``
     Using directive precedes header inclusion.

   Will not warn about packages included in parameter ``global_packages``
   (default ``bslmf bslstl``).

.. only:: bde_verify or bb_cppverify

   verify-same-argument-names
   ++++++++++++++++++++++++++

   The declaration and definition of a function should use the same names for
   the function parameters.

   * ``AN01``
     Function declaration and definition use different parameter names.

.. only:: bde_verify or bb_cppverify

   whitespace
   ++++++++++

   Whitespace problems.

   * ``TAB01``
     File contains tab characters.
   * ``ESP01``
     File contains spaces at end of lines.

Building |bv|
-------------
See the README file at the top level of the source tree.

..
   ----------------------------------------------------------------------------
   Copyright (C) 2015 Bloomberg Finance L.P.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
   
       http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   ----------------------------- END-OF-FILE ----------------------------------
