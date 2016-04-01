.. bde-verify documentation master file, created by
   sphinx-quickstart on Wed Apr  8 17:21:22 2015.

|BV|
====

.. toctree::
   :maxdepth: 2

Name
----
|bv| - static C++-compiler-based checking tool

Synopsis
--------
|bv| ``[options...] [compiler options...] file...``

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

If the ``--rewrite=dir`` option is specified, |bv| will make some suggested
changes by itself and place the modified files in the specified directory with
``-rewritten`` appended to the file names. (Not much rewriting is being done
yet; we plan to increase this over time.)

|Bv| is supported on SunOS and Linux. In the BloombergLP development
environment, running ``/opt/bb/bin/``\ |bv| will launch the appropriate
version.

Options
-------
--config file         configuration file
--cl line             additional configuration lines
--bb dir              base directory for |BDE| header includes
--exe binary          bde_verify executable file
--cc compiler         C++ compiler used to find system include directories
--definc              set up default include paths
--nodefinc            do not set up default include paths
--defdef              set up default macro definitions
--nodefdef            do not set up default macro definitions
--ovr                 define BSL_OVERRIDES_STD
--noovr               do not define BSL_OVERRIDES_STD
--rewrite-dir dir     place rewritten files (as name-rewritten) in dir
--rewrite dir         (same as --rewrite-dir)
--rd dir              (same as --rewrite-dir)
--rewrite-file file   accumulate rewrite specifications into file
--rf file             (same as --rewrite-file)
--std type            specify C++ version
--tag string          make first line of each warning contain [string]
--diagnose type       report and rewrite only for main, component, nogen, or all
--m32                 process in 32-bit mode
--m64                 process in 64-bit mode
--nsa                 allow logging for purposes of tracking usage
--nonsa               disallow logging for purposes of tracking usage
--debug               display internal information as checks are performed
--verbose             display command line passed to clang
-v                    (same as --verbose)
--help                display this help message (also -?)
-I directory          add directory to header search path
-D macro              define macro
-W warning            enable warning
-f flag               specify compiler flag
-w                    disable normal compiler warnings

Configuration
-------------
The configuration file allows individual or groups of checks to enabled or
disabled, and specifies the enterprise namespace in which components live. By
default, that namespace is ``BloombergLP``, and almost all checks are enabled.
The configuration file consists of a set of options, one per line, processed
in order. Additional configuration lines may be supplied on the command line
as described above. In particular, specifying ``-cl='load file'`` will
augment the default configuration with the contents of ``file``.

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
``set`` *parameter* *value*     Set a parameter used by a check.
``append`` *parameter* *value*  Append to a parameter used by a check.
``prepend`` *parameter* *value* Prepend to a parameter used by a check.
``suppress`` *tag* *files*...   Messages with the specified *tag* are
                                suppressed for the specified *files*. Either
                                *tag* or *files* (but not both) may be ``*``.
                                The *tag* may be a group *name*, suppressing
                                all members (including subgroups).
=============================== ===============================================

If the configuration file attempts to name a non-existent check, the tool will 
report a list of all known checks and then exit. Do this deliberately to 
obtain an accurate list of checks if you suspect this documentation is out of 
date.

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

Exit Status
-----------
Normally, the exit status of a |bv| run is 0 (success) unless the code has
actual errors.  If a particular check or tag is produced and that check or tag
is set in the *failstatus* configuration parameter, the exit status will be 1
(failure).  This allows for the creation of wrapper scripts whose exit status
indicates that some condition fails to hold.

Checks
------
These are the checks supported by the tool. A few are of dubious value and may
be removed in the future. The tag prefixes (especially the ``TR``\ *nn* ones)
are subject to change as tests are refined or updated. We welcome suggestions
for additional checks.

.. only:: bde_verify or bb_cppverify

   ``allocator-forward``
   +++++++++++++++++++++
   Checks dealing with allocator forwarding and traits.

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

.. only:: bde_verify or bb_cppverify

   ``allocator-new``
   +++++++++++++++++
   * ``ANP01``
     Calls to placement new with an argument that is a pointer to an allocator.

.. only:: bde_verify

   ``alphabetical-functions``
   ++++++++++++++++++++++++++
   * ``FABC01``
     Functions in a component section that are not in alphanumeric order.

   Note that the ordering resets in certain cases, such as when a pair of
   functions are not from the same context.

   Ordering also resets across single-line comments such as
   ``// CLASS METHODS`` and line banners.

.. only:: bde_verify or bb_cppverify

   ``anon-namespace``
   ++++++++++++++++++
   * ``ANS01``
     Anonymous namespace in header.

.. only:: bde_verify or bb_cppverify

   ``array-argument``
   ++++++++++++++++++
   * ``AA01``
     Sized array parameter is really a pointer.

.. only:: bde_verify

   ``array-initialization``
   ++++++++++++++++++++++++
   * ``II01``
     Incomplete array initialization in which the last value is not the default
     member value.

.. only:: bde_verify or bb_cppverify

   ``assert-assign``
   ++++++++++++++++++++++++
   * ``AE01``
     Top-level macro condintion is an assignment.

.. only:: bde_verify

   ``banner``
   ++++++++++
   Malformed banners.

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

   ``base``
   ++++++++
   * ``PR01``
     ``#pragma`` |bv| ``pop`` when stack is empty.
   * ``PR02``
     ``#pragma`` |bv| ``push`` is never popped.

.. only:: bde_verify

   ``boolcomparison``
   ++++++++++++++++++
   * ``BC01``
     Comparison of a Boolean expression with literal ``true`` or ``false``.

.. only:: bde_verify

   ``bsl-overrides-std``
   +++++++++++++++++++++
   Rewrite code which compiles with ``BSL_OVERRIDES_STD`` defined to not
   require that.
   Use the ``-rewrite`` option to generate the rewritten file.

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

   ``bsl-std-string``
   ++++++++++++++++++
   * ``ST01``
     Converting std::string to bsl::string.
   * ``ST02``
     Converting bsl::string to std::string.

.. only:: bde_verify or bb_cppverify

   ``c-cast``
   ++++++++++
   * ``CC01``
     C-style cast expression. (Dispensation is granted to ``(void)expr``.)

.. only:: bde_verify or bb_cppverify

   ``char-classification-range``
   +++++++++++++++++++++++++++++
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

   ``char-vs-string``
   ++++++++++++++++++
   * ``ADC01``
     Passing the address of a single character as an argument to a
     ``const char *`` parameter.

.. only:: bde_verify

   ``class-sections``
   ++++++++++++++++++
   BDE coding standards require that class member declarations appear in tagged
   sections (e.g., ``// MANIPULATORS``, ``// CREATORS``, et al.)

   * ``KS00``
     Declaration without tag.
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

.. only:: bde_verify

   ``comments``
   ++++++++++++
   Comments containing erroneous or deprecated text.

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

   ``comparison-order``
   ++++++++++++++++++++
   Comparisons whose operand order should be reversed.

   * ``CO01``
     Non-modifiable operand should be on the left.
   * ``CO02``
     Constant-expression operand should be on the left.

.. only:: bde_verify or bb_cppverify

   ``component-header``
   ++++++++++++++++++++
   * ``TR09``
     Component implementation file does not include its header file ahead of
     other includes or declarations.

.. only:: bde_verify or bb_cppverify

   ``component-prefix``
   ++++++++++++++++++++
   * ``CP01``
     Globally visible name is not prefixed by component name.

   Will not warn about packages included in parameter ``global_packages``
   (default ``bslmf bslstl``).

.. only:: bde_verify

   ``constant-return``
   +++++++++++++++++++
   * ``CR01``
     Single statement function returns a constant value.

.. only:: bde_verify

   ``contiguous-switch``
   +++++++++++++++++++++
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

.. only:: bde_verify or bb_cppverify

   ``cpp-in-extern-c``
   +++++++++++++++++++
   Header files with C++ constructs included within ``extern "C"`` contexts.

   * ``PC01``
     C++ header included within C linkage specification.

.. only:: bde_verify or bb_cppverify

   ``deprecated``
   ++++++++++++++
   * ``DP01``
     Call to deprecated function.

.. only:: bde_verify

   ``diagnostic-filter``
   +++++++++++++++++++++
   Not a check.

.. only:: bde_verify or bb_cppverify

   ``do-not-use-endl``
   +++++++++++++++++++++++
   * ``NE01``
     Prefer using ``'\\n'`` over ``endl``.

.. only:: bde_verify

   ``dump-ast``
   +++++++++++++++++++++++
   Not a check.

.. only:: bde_verify or bb_cppverify

   ``entity-restrictions``
   +++++++++++++++++++++++
   * ``TR17``
     Items declared in global scope.

.. only:: bde_verify

   ``enum-value``
   ++++++++++++++
   * ``EV01``
     Component enumeration tag is ``Value``.

.. only:: bde_verify or bb_cppverify

   ``external-guards``
   +++++++++++++++++++
   Incorrect or missing use of external header guards.

   * ``SEG01``
     Include guard without include file.
   * ``SEG02``
     Include guard does not match include file.
   * ``SEG03``
     File included in header without include guard test.

.. only:: bde_verify

   ``files``
   +++++++++
   Missing or inaccessible component header file or test driver.

   * ``FI01``
     Component header file is missing.
   * ``FI02``
     Component test driver file is missing.

.. only:: bde_verify or bb_cppverify

   ``free-functions-depend``
   +++++++++++++++++++++++++

   * ``AQS01``
     Free function parameter must depend on a local definition.

.. only:: bde_verify or bb_cppverify

   ``friends-in-headers``
   ++++++++++++++++++++++

   * ``AQP01``
     Friends must be declared in the same header.

.. only:: bde_verify

   ``function-contract``
   +++++++++++++++++++++
   Incorrect or missing function contracts.

   * ``FD01``
     Missing contract.
   * ``FD02``
     Contract indented incorrectly.
   * ``FD03``
     Parameter is not documented.
   * ``FD04``
     Parameter name is not single-quoted.
   * ``FD05``
     Parameters with default values are not called out with
     *optionally specify*.
   * ``FD06``
     Parameters are not called out with *specified*.
   * ``FD07``
     Parameter called out with *specified* more than once.

.. only:: bde_verify or bb_cppverify

   ``global-data``
   +++++++++++++++
   * ``AQb01``
     Data variable with global visibilty.

.. only:: bde_verify or bb_cppverify

   ``global-function-only-in-source``
   ++++++++++++++++++++++++++++++++++
   * ``TR10``
     Globally visible function not declared in header.

.. only:: bde_verify or bb_cppverify

   ``global-type-only-in-source``
   ++++++++++++++++++++++++++++++
   * ``TR10``
     Globally visible type not declared in header.
   * ``TR11``
     Globally visible type should be defined in header.

.. only:: bde_verify

   ``groupname``
   +++++++++++++
   Component is not properly named or located.

   * ``GN01``
     Component does not have a distinguishable correctly formed package group
     name.
   * ``GN02``
     Component is not located within its correct package group directory.

.. only:: bde_verify or bb_cppverify

   ``hash-pointer``
   ++++++++++++++++
   * ``HC01``
     Warn that use of ``std::hash<TYPE*>()(ptr)`` uses only the value and not
     the contents of *ptr*.

.. only:: bde_verify

   ``headline``
   ++++++++++++
   * ``HL01``
     The headline of the file is incorrect.

.. only:: bde_verify

   ``implicit-ctor``
   +++++++++++++++++
   * ``IC01``
     Non-``explicit`` constructor which may be invoked implicitly and
     not marked with ``// IMPLICIT``

.. only:: bde_verify or bb_cppverify

   ``in-enterprise-namespace``
   +++++++++++++++++++++++++++
   * ``AQQ01``
     Declaration not in enterprise namespace.

.. only:: bde_verify or bb_cppverify

   ``include-guard``
   +++++++++++++++++
   * ``TR14``
     Header file does not set up or use its include guard macro properly.

.. only:: bde_verify

   ``include-in-extern-c``
   +++++++++++++++++++++++
   * ``IEC01``
     Header file included within C linkage specification.

.. only:: bde_verify

   ``include-order``
   +++++++++++++++++
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

   ``indentation``
   +++++++++++++++
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

   ``leaking-macro``
   +++++++++++++++++
   * ``SLM01``
     Component header file macro neither an include guard nor prefixed by
     component name.

.. only:: bde_verify or bb_cppverify

   ``local-friendship-only``
   +++++++++++++++++++++++++
   Long-distance friendship.

   * ``TR19``
     Friendship granted outside of component.

.. only:: bde_verify

   ``long-inline``
   +++++++++++++++
   * ``LI01``
     Inline function is longer than configuration file parameter
     ``max_inline_lines`` (default 10).

.. only:: bde_verify

   ``longlines``
   +++++++++++++
   * ``LL01``
     Line exceeds 79 characters.

.. only:: bde_verify

   ``member-definition-in-class-definition``
   +++++++++++++++++++++++++++++++++++++++++
   * ``CD01``
     Method defined directly in class definition.

.. only:: bde_verify or bb_cppverify

   ``member-names``
   ++++++++++++++++
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

   ``mid-return``
   ++++++++++++++
   * ``MR01``
     Non-final ``return`` statement not tagged with ``// RETURN``.
   * ``MR02``
     ``// RETURN`` tag does not end in column 79.

.. only:: bde_verify

   ``namespace-tags``
   ++++++++++++++++++
   * ``NT01``
     Multi-line namespace blocks must end with
     ``// close [ enterprise | package | unnamed | description ] namespace``.

.. only:: bde_verify

   ``nested-declarations``
   +++++++++++++++++++++++
   * ``TR04``
     Declarations not properly nested in package namespace.

     Will not warn about main files unless parameter ``main_namespace_check``
     is ``on`` (default ``off``).

     Will not warn about packages included in parameter ``global_packages``
     (default ``bslmf bslstl``).

.. only:: bde_verify or bb_cppverify

   ``nonascii``
   ++++++++++++
   * ``NA01``
     Source code contains bytes with value greater than 127.

.. only:: bde_verify

   ``operator-void-star``
   ++++++++++++++++++++++
   * ``CB01``
     Class contains conversion operator to ``void *`` or ``bool``.

.. only:: bde_verify

   ``packagename``
   +++++++++++++++
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

   ``ref-to-movableref``
   +++++++++++++++++++++
   * ``MRR01``
     MovableRef should be passed by value, not reference.

.. only:: bde_verify

   ``refactor``
   ++++++++++++
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
   * ``RD0111
     Replacing forward class declaration.

.. only:: bde_verify

   ``refactor-config``
   +++++++++++++++++++
   Given pairs of old/new header files, generate a configuration file for the
   ``refactor`` check from corresponding pairs of names appending to the file
   specified by the configuration file parameter ``refactorfile`` (or the
   default, "refactor.cfg" if left unspecified).

   * ``DD01``
     Eligible name for refactoring.

.. only:: bde_verify or bb_cppverify

   ``runtime-initialization``
   ++++++++++++++++++++++++++
   * ``AQa01``
     Global variable with runtime initialization.

.. only:: bde_verify

   ``spell-check``
   +++++++++++++++
   * ``SP01``
     Misspelled word in comment.
   * ``SP02``
     Cannot start spell checker.  (Not an error in the examined file.)
   * ``SP03``
     Misspelled word in parameter name.

   Spell-checking is disabled by default in the config file
   (``check spell-check off``) to avoid noise.

   Words in configuration parameter ``dictionary`` (default too numerous to
   mention - see config file) are assumed correct.

   Words that appear at least as many times as non-zero configuration
   parameter ``spelled_ok_count`` (default 3) are assumed correct.

   The spell checker is the library version of `GNU Aspell`_.

   .. _GNU Aspell: http://aspell.net

.. only:: bde_verify or bb_cppverify

   ``strict-alias``
   ++++++++++++++++
   * ``AL01``
     Possible strict-aliasing violation.

.. only:: bde_verify or bb_cppverify

   ``string-add``
   ++++++++++++++
   * ``SA01``
     Addition of integer and string literal.

.. only:: bde_verify

   ``swap-a-b``
   ++++++++++++
   * ``SWAB01``
     Parameters of free *swap* function are not named *a* and *b*.

.. only:: bde_verify or bb_cppverify

   ``swap-using``
   ++++++++++++++
   Directly invoking ``std::swap`` or ``bsl::swap`` can prevent argument-
   dependent lookup from finding overloads.

   * ``SU01``
     Prefer ``using std::swap; swap(...);'`` over ``std::swap(...);``.

.. only:: bde_verify

   ``template-typename``
   +++++++++++++++++++++
   * ``TY01``
     Use of ``typename`` instead of ``class`` in ``template`` header.
   * ``TY02``
     Use of single-letter template parameter names.
   * ``TY03``
     Use of non ``ALL_CAPS`` template parameter names.

.. only:: bde_verify

   ``test-driver``
   +++++++++++++++
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

   ``that-which``
   +++++++++++++++++++++++++++
   * ``TW01``
     Prefer ``that`` to ``which``.
   * ``TW02``
     Possibly incorrect comma before ``that``.

.. only:: bde_verify or bb_cppverify

   ``throw-non-std-exception``
   +++++++++++++++++++++++++++
   * ``FE01``
     Throwing exception not derived from ``std::exception``.

.. only:: bde_verify or bb_cppverify

   ``transitive-includes``
   +++++++++++++++++++++++++++
   * ``AQK01``
     Header included transitively should be included directly.

.. only:: bde_verify or bb_cppverify

   ``unnamed-temporary``
   +++++++++++++++++++++
   * ``UT01``
     Unnamed object will be immediately destroyed.

   The canonical example of this error is ``mutex m; mutex_guard(&m);``.

.. only:: bde_verify

   ``upper-case-names``
   ++++++++++++++++++++
   * ``UC01``
     Names of variables and types should not be all upper-case.

.. only:: bde_verify or bb_cppverify

   ``using-declaration-in-header``
   +++++++++++++++++++++++++++++++
   * ``TR16``
     Header file contains ``using`` declaration.
   * ``AQJ01``
     Using declaration precedes header inclusion.

   Will not warn about packages included in parameter ``global_packages``
   (default ``bslmf bslstl``).

.. only:: bde_verify or bb_cppverify

   ``using-directive-in-header``
   +++++++++++++++++++++++++++++
   * ``TR16``
     Header file contains ``using`` directive.
   * ``AQJ02``
     Using directive precedes header inclusion.

   Will not warn about packages included in parameter ``global_packages``
   (default ``bslmf bslstl``).

.. only:: bde_verify or bb_cppverify

   ``verify-same-argument-names``
   ++++++++++++++++++++++++++++++
   * ``AN01``
     Function declaration and definition use different parameter names.

.. only:: bde_verify or bb_cppverify

   ``whitespace``
   ++++++++++++++
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
  
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
  
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
  
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
   ----------------------------- END-OF-FILE ----------------------------------
