//===-- BdeFormatPackages.cs - VSPackage for bde-verify ---------*- C# -*-===//
//
//===----------------------------------------------------------------------===//
//
// This class contains a VS extension package that runs bde-verify over a
// VS text editor buffer.
//
//===----------------------------------------------------------------------===/

using EnvDTE;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using System;
using System.Collections;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml.Linq;
using System.Linq;

using Microsoft.Win32;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.TextManager.Interop;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;

namespace BloombergLP.BdeVerify {
    /// <summary>
    /// An enumeration specifying for which files included in the compilation
    /// unit diagnostics should be emitted.
    /// </summary>
    public enum DiagnoseWhat {
        /// <summary>
        /// Diagnose within the file specified.
        /// </summary>
        main,

        /// <summary>
        /// Diagnose within the file specified, its header, and its test driver.
        /// </summary>
        component,

        /// <summary>
        /// Diagnose in all files except for automatically generated ones.
        /// </summary>
        nogen,

        /// <summary>
        /// Diagnose in all files.
        /// </summary>
        all,
    };

    [ClassInterface(ClassInterfaceType.AutoDual)]
    [CLSCompliant(false), ComVisible(true)]
    public class OptionPageGrid : DialogPage {
        private string config = "";
        [Category("BDE Verify")]
        [DisplayName("Config File")]
        [Description("File containing bde-verify configuration options.")]
        public string Config { get { return config; } set { config = value; } }

        private string exe = "";
        [Category("BDE Verify")]
        [DisplayName("Program File")]
        [Description("BDE Verify executable binary.")]
        public string Exe { get { return exe; } set { exe = value; } }

        private bool off = false;
        [Category("BDE Verify")]
        [DisplayName("Off")]
        [Description("Disable all checks.")]
        public bool Off { get { return off; } set { off = value; } }

        private Collection<string> extra = new Collection<string>();
        [Category("BDE Verify")]
        [DisplayName("Additional Configuration")]
        [Description("Extra configuration lines.")]
        public Collection<string> Extra { get { return extra; } set { extra = value; } }

        private DiagnoseWhat diagnose = DiagnoseWhat.component;
        [Category("BDE Verify")]
        [DisplayName("Diagnose")]
        [Description("Files in the compilation unit for which diagnostics should be emitted.")]
        public DiagnoseWhat Diagnose { get { return diagnose; } set { diagnose = value; } }

        private bool verbose = false;
        [Category("BDE Verify")]
        [DisplayName("Verbose")]
        [Description("Show the command line used to invoke BDE Verify.")]
        public bool Verbose { get { return verbose; } set { verbose = value; } }
    }

    /// <summary>
    /// This is the class that implements the package exposed by this assembly.
    ///
    /// The minimum requirement for a class to be considered a valid package for Visual Studio
    /// is to implement the IVsPackage interface and register itself with the shell.
    /// This package uses the helper classes defined inside the Managed Package Framework (MPF)
    /// to do it: it derives from the Package class that provides the implementation of the 
    /// IVsPackage interface and uses the registration attributes defined in the framework to 
    /// register itself and its components with the shell.
    /// </summary>
    // This attribute tells the PkgDef creation utility (CreatePkgDef.exe) that this class is
    // a package.
    [PackageRegistration(UseManagedResourcesOnly = true)]
    // This attribute is used to register the information needed to show this package
    // in the Help/About dialog of Visual Studio.
    [InstalledProductRegistration("#110", "#112", "1.0", IconResourceID = 400)]
    // This attribute is needed to let the shell know that this package exposes some menus.
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideAutoLoad(UIContextGuids80.SolutionExists)] // Load package on solution load
    [Guid(GuidList.guidBdeVerifyPkgString)]
    [ProvideOptionPage(typeof(OptionPageGrid), "BDE", "BdeVerify", 0, 0, true)]
    public sealed class BdeVerifyPackage : Package {
        #region Package Variables
        /// <summary>
        /// Set of error-list providers mapped by top-level file name as passed to bde_verify.
        /// These providers persist so that their results are visible in the error list, and
        /// their errors are first cleared each time bde_verify is run against their file.
        /// </summary>
        Dictionary<string, ErrorListProvider> elps;

        /// <summary>
        /// The GUID of the enumeration literal EnvDTE.Constants.vsViewKindCode.
        /// </summary>
        Guid gvkc;

        /// <summary>
        /// A regular expression matching the lead line of a bde_verify message,
        ///    file:line:column: messagetype: message
        /// </summary>
        Regex rx;

        /// <summary>
        /// A regular expression matching the end of bde_verify output,
        ///    NNN warnings generated.
        /// </summary>
        Regex re;
        #endregion

        /// <summary>
        /// Default constructor of the package.
        /// Inside this method you can place any initialization code that does not require 
        /// any Visual Studio service because at this point the package object is created but 
        /// not sited yet inside Visual Studio environment. The place to do all the other 
        /// initialization is the Initialize method.
        /// </summary>
        public BdeVerifyPackage() {
            elps = new Dictionary<string, ErrorListProvider>();
            rx = new Regex("^(?<file>.*):(?<line>[0-9]+):(?<column>[0-9]+): ((?<error>error)|(?<warning>warning)|(?<note>note)): (?<message>.*)$");
            re = new Regex("^[0-9] .* generated[.]$");
        }

        /////////////////////////////////////////////////////////////////////////////
        // Overridden Package Implementation
        #region Package Members

        /// <summary>
        /// Initialization of the package; this method is called right after the package is sited, so this is the place
        /// where you can put all the initialization code that rely on services provided by VisualStudio.
        /// </summary>
        protected override void Initialize() {
            gvkc = new Guid(EnvDTE.Constants.vsViewKindCode);

            base.Initialize();

            // Add our command handlers for menu (commands must exist in the .vsct file)
            var mcs = GetService(typeof(IMenuCommandService)) as OleMenuCommandService;
            if (null != mcs) {
                // Create the command for the menu item.
                var menuCommandID = new CommandID(GuidList.guidBdeVerifyCmdSet, (int)PkgCmdIDList.cmdidBVCMD);
                var menuItem = new MenuCommand(MenuItemCallback, menuCommandID);
                mcs.AddCommand(menuItem);
            }
        }
        #endregion

        OptionPageGrid GetUserOptions()
        {
            return (OptionPageGrid)GetDialogPage(typeof(OptionPageGrid));
        }

        /// <summary>
        /// This function is the callback used to execute a command when the a menu item is clicked.
        /// See the Initialize method to see how the menu item is associated to this function using
        /// the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        private void MenuItemCallback(object sender, EventArgs e) {
            string message = null;
            OptionPageGrid options = GetUserOptions();
            // If command is invoked on a non-C++ buffer, or there's no project, and so forth, some
            // of the data structure traversals below will encounter null dereferences.  Rather than
            // testing for null everywhere, we just handle the exception and bail.
            try {
                var dte = GetService(typeof(DTE)) as DTE;
                var ivs = GetGlobalService(typeof(IVsSolution)) as IVsSolution;
                var textManager = Package.GetGlobalService(typeof(SVsTextManager)) as IVsTextManager;
                IVsTextView textView;
                textManager.GetActiveView(1, null, out textView);
                var userData = textView as IVsUserData;
                var guidWpfViewHost = DefGuidList.guidIWpfTextViewHost;
                object host;
                userData.GetData(ref guidWpfViewHost, out host);
                message = "File must be part of a project";
                var document = (host as IWpfTextViewHost).TextView.TextBuffer.Properties.GetProperty(typeof(ITextDocument)) as ITextDocument;
                var project = dte.Solution.FindProjectItem(document.FilePath).ContainingProject;
                dynamic vcproject = project.Object;
                var configuration = vcproject.Configurations.Item(dte.Solution.SolutionBuild.ActiveConfiguration.Name);
                message = null;
                dynamic vctool = configuration.Tools.Item("VCCLCompilerTool");
                var process = new System.Diagnostics.Process();
                var si = process.StartInfo;
                si.UseShellExecute = false;
                si.FileName = FindExecutable(options.Exe, "bde_verify_bin.exe");
                if (options.Exe == "") {
                    options.Exe = si.FileName;
                }
                string cfg = FindConfigFile(options.Config, si.FileName, "bde_verify.cfg");
                si.CreateNoWindow = true;
                si.RedirectStandardInput = false;
                si.RedirectStandardOutput = false;
                si.RedirectStandardError = true;
                si.WorkingDirectory = Directory.GetParent(document.FilePath).ToString();
                si.Arguments += xclang("-plugin") + xclang("bde_verify");
                if (cfg != null) {
                    si.Arguments += plugin("config=" + Quote(cfg));
                    if (options.Config == "") {
                        options.Config = cfg;
                    }
                }
                if (options.Off) {
                    si.Arguments += plugin("config-line=" + Quote("all off"));
                }
                foreach (string s in options.Extra) {
                    si.Arguments += plugin("config-line=" + Quote(s));
                }
                si.Arguments += plugin("diagnose=" + options.Diagnose);
                si.Arguments += " -fcxx-exceptions";
                si.Arguments += " -fexceptions";
                si.Arguments += " -fdiagnostics-show-note-include-stack";
                si.Arguments += " -fdiagnostics-show-option";
                si.Arguments += " -fsyntax-only";
                si.Arguments += " -ferror-limit=0";
                si.Arguments += " -fms-compatibility";
                si.Arguments += " -fms-extensions";
                si.Arguments += " -std=c++11";
                si.Arguments += " -xc++";
                var paths = new HashSet<string>();
                paths.Add("");
                message = "Cannot find preprocessor include path";
                foreach (string dir in (".;" + vctool.FullIncludePath).Split(';')) {
                    if (paths.Add(dir)) {
                        si.Arguments += " -isystem " + Quote(dir);
                    }
                }
                message = null;
                var defs = new HashSet<string>();
                defs.Add("");
                message = "Cannot find preprocessor definitions";
                foreach (string def in ("BDE_VERIFY;" + vctool.PreprocessorDefinitions).Split(';')) {
                    if (defs.Add(def)) {
                        si.Arguments += " -D" + Quote(def);
                    }
                }
                message = null;
                si.Arguments += " " + Quote(document.FilePath);
                try {
                    process.Start();
                    if (!elps.ContainsKey(document.FilePath)) {
                        elps[document.FilePath] = new ErrorListProvider(this);
                    }
                    var elp = elps[document.FilePath];
                    IVsHierarchy h;
                    ivs.GetProjectOfUniqueName(project.UniqueName, out h);
                    ErrorTask msg = new ErrorTask {
                        Category = TaskCategory.BuildCompile,
                        ErrorCategory = TaskErrorCategory.Message,
                        HierarchyItem = h,
                        Document = document.FilePath,
                        Line = 0,
                        Column = 0,
                        Text = "BDE Verify in progress...",
                    };
                    elp.Tasks.Clear();
                    elp.Tasks.Add(msg);
                    elp.Show();
                    ErrorTask et = null;
                    bool last = false;
                    process.ErrorDataReceived += (object p, DataReceivedEventArgs d) => {
                        if (d.Data == null) {
                            if (!last) {
                                msg.Text = "BDE Verify done";
                                elp.Refresh();
                            }
                            return;
                        }
                        Debug.Print(d.Data);
                        Match m = rx.Match(d.Data);
                        last = re.IsMatch(d.Data);
                        if (last) {
                            msg.Text = "BDE Verify done: " + d.Data;
                            elp.Refresh();
                        }
                        if (et != null && (m.Success || last)) {
                            elp.Tasks.Add(et);
                            elp.Show();
                        }
                        if (m.Success) {
                            et = new ErrorTask {
                                Category = TaskCategory.BuildCompile,
                                ErrorCategory =
                                    m.Groups["warning"].Success ? TaskErrorCategory.Warning :
                                    m.Groups["error"].Success ? TaskErrorCategory.Error :
                                                             TaskErrorCategory.Message,
                                HierarchyItem = h,
                                Line = Convert.ToInt32(m.Groups["line"].Value) - 1,
                                Column = Convert.ToInt32(m.Groups["column"].Value) - 1,
                                Document = m.Groups["file"].Value,
                                Text = m.Groups["message"].Value,
                            };
                            et.Navigate += (ss, ee) => {
                                var src = ss as ErrorTask;
                                src.Line++; src.Column++;
                                elp.Navigate(src, gvkc);
                                src.Line--; src.Column--;
                            };
                        } else if (!last && et != null) {
                            et.Text += "\n" + d.Data;
                        }
                    };
                    process.BeginErrorReadLine();
                    if (options.Verbose) {
                        message = (Quote(si.FileName) + " " + si.Arguments).Replace(" -", "\n-");
                    }
                } catch (Exception x) {
                    message = "Error: " + process.StartInfo.FileName + "\n" + x.Message;
                }
            } catch (NullReferenceException x) {
                if (message == null) {
                    message = x.Message;
                }
            }
            if (message != null) {
                IVsUIShell uiShell = GetService(typeof(SVsUIShell)) as IVsUIShell;
                Guid clsid = Guid.Empty;
                int result;
                Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(uiShell.ShowMessageBox(
                           0,
                           ref clsid,
                           "BdeVerify",
                           string.Format(CultureInfo.CurrentCulture, "{0}\n{1}", this.ToString(), message),
                           string.Empty,
                           0,
                           OLEMSGBUTTON.OLEMSGBUTTON_OK,
                           OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST,
                           OLEMSGICON.OLEMSGICON_INFO,
                           0,        // false
                           out result));
            }
        }

        /// <summary>
        /// If <paramref name="s"/> contains spaces or double quotes, return it wrapped in
        /// double quotes with internal double quotes doubled.  Otherwise just return
        /// <paramref name="s"/> itself.
        /// </summary>
        /// <param name="s">The string to be wrapped.</param>
        /// <returns>The input string, wrapped if necessary.</returns>
        private static string Quote(string s) {
            if (s.Contains(" ") | s.Contains("\"")) {
                s = "\"" + s.Replace("\"", "\"\"") + "\"";
            }
            return s;
        }

        /// <summary>
        /// Return <paramref name="s"/> prepended with " -Xclang ".
        /// </summary>
        /// <param name="s">The string to be prepended with " -Xclang ".</param>
        /// <returns>The input string prepended with " -Xclang ".</returns>
        private static string xclang(string s) {
            return " -Xclang " + s;
        }

        /// <summary>
        /// Return <paramref name="s"/> prepended with " -Xclang -plugin-arg-bde_verify -Xclang ".
        /// </summary>
        /// <param name="s">The string to be prepended with " -Xclang -plugin-arg-bde_verify -Xclang ".</param>
        /// <returns>The input string prepended with " -Xclang -plugin-arg-bde_verify -Xclang ".</returns>
        private static string plugin(string s) {
            return xclang("-plugin-arg-bde_verify") + xclang(s);
        }

        /// <summary>
        /// Given the base <paramref name="name"/> of an executable, find its full path.
        /// In order, look for <paramref name="exe"/> from the options page, expanding
        /// its environment variables is given, then for <paramref name="name"/> in App
        /// Paths in HKCU and HKLM, then in directories in PATH, and finally in a default
        /// location.
        /// </summary>
        /// <param name="exe">The executable specified in the options page".</param>
        /// <param name="name">The base name of the executable, "bde_verify_bin.exe".</param>
        /// <returns>The found executable file, or a default name.</returns>
        private static string FindExecutable(string exe, string name) {
            string result = null;
            try {
                result = Path.GetFullPath(Environment.ExpandEnvironmentVariables(exe));
            } catch { }
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                result = Path.GetDirectoryName(typeof(BdeVerifyPackage).Assembly.Location) + "\\bde_verify_bin.exe";
            }
            string apkey = Path.Combine(new string[] {
                    "SOFTWARE", "Microsoft", "Windows", "CurrentVersion", "App Paths", name
            });
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                var reg = Registry.GetValue(Path.Combine("HKEY_CURRENT_USER", apkey), null, null);
                if (reg != null) {
                    result = reg.ToString();
                }
            }
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                var reg = Registry.GetValue(Path.Combine("HKEY_LOCAL_MACHINE", apkey), null, null);
                if (reg != null) {
                    result = reg.ToString();
                }
            }
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                foreach (var dir in Environment.GetEnvironmentVariable("PATH").Split(';')) {
                    string file = Path.Combine(dir, name);
                    if (File.Exists(file)) {
                        result = file;
                        break;
                    }
                }
            }
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                result = Path.Combine(new string[] {
                        Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),
                        "BDE Verify", "libexec", "bde-verify", name
                    });
            }
            return result;
        }

        /// <summary>
        /// Find the config file to use, based on <paramref name="cfg"/>, from the Options
        /// Page, <paramref name="exe"/>, the full path name of the executable, and
        /// <paramref name="name"/>, the desired name of the config fiule.  If the file
        /// specified in the options is found, it is used, otherwise given an executable of
        /// .../base/libexec/bde-verify/<paramref name="exe"/>, we look in
        /// .../base/etc/bde-verify/<paramref name="name"/>.
        /// </summary>
        /// <param name="cfg"></param>
        /// <param name="exe"></param>
        /// <param name="name"></param>
        /// <returns>The located configuration file or null.</returns>
        private static string FindConfigFile(string cfg, string exe, string name) {
            string result = null;
            try {
                result = Path.GetFullPath(Environment.ExpandEnvironmentVariables(cfg));
            } catch { }
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                result = Path.Combine(
                    Path.GetDirectoryName(Path.GetDirectoryName(Path.GetDirectoryName(exe))),
                    "etc", "bde-verify", name);
            }
            if (String.IsNullOrEmpty(result) || !File.Exists(result)) {
                return null;
            }
            return result;
        }
    }
}

// ----------------------------------------------------------------------------
// Copyright (C) 2018 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
