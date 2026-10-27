using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

public sealed class NfsLocateClang : Task
{
    [Required] public string ProjectDirectory { get; set; }
    [Required] public string RecordDirectory { get; set; }
    public string VisualCppDirectory { get; set; }
    public string SearchPath { get; set; }
    public ITaskItem[] DocumentsDirectories { get; set; }
    [Output] public string CompilerDirectory { get; set; }
    [Output] public string CompilerArchitecture { get; set; }
    [Output] public string CompilerSignature { get; set; }
    [Output] public ITaskItem[] CompilerFile { get; set; }
    [Output] public bool CompilerChanged { get; set; }

    private readonly HashSet<string> checkedFiles = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
    private string executable;

    private bool TryCompiler(string candidate)
    {
        try
        {
            if (String.IsNullOrWhiteSpace(candidate))
                return false;
            string path = Path.GetFullPath(Environment.ExpandEnvironmentVariables(candidate.Trim().Trim('"')));
            if (!checkedFiles.Add(path) || !String.Equals(Path.GetFileName(path), "clang-cl.exe", StringComparison.OrdinalIgnoreCase) || !File.Exists(path))
                return false;
            using (var stream = File.OpenRead(path))
            using (var reader = new BinaryReader(stream))
            {
                if (stream.Length < 64 || reader.ReadUInt16() != 23117)
                    return false;
                stream.Position = 60;
                uint header = reader.ReadUInt32();
                if (header > stream.Length - 6)
                    return false;
                stream.Position = header;
                if (reader.ReadUInt32() != 17744)
                    return false;
                ushort machine = reader.ReadUInt16();
                if (machine != 332 && machine != 34404)
                    return false;
                CompilerArchitecture = machine == 34404 ? "Native64Bit" : "Native32Bit";
            }
            executable = path;
            return true;
        }
        catch (IOException) { return false; }
        catch (UnauthorizedAccessException) { return false; }
        catch (ArgumentException) { return false; }
        catch (System.Security.SecurityException) { return false; }
        catch (NotSupportedException) { return false; }
    }

    private bool TryBin(string directory)
    {
        return !String.IsNullOrWhiteSpace(directory) && TryCompiler(Path.Combine(directory, "clang-cl.exe"));
    }

    private bool TryRegistry()
    {
        if (Environment.OSVersion.Platform != PlatformID.Win32NT)
            return false;
        Type keyType = Type.GetType("Microsoft.Win32.RegistryKey, Microsoft.Win32.Registry", false)
            ?? Type.GetType("Microsoft.Win32.RegistryKey", false);
        if (keyType == null)
            return false;
        Type hiveType = keyType.Assembly.GetType("Microsoft.Win32.RegistryHive");
        Type viewType = keyType.Assembly.GetType("Microsoft.Win32.RegistryView");
        if (hiveType == null || viewType == null)
            return false;
        MethodInfo openBase = keyType.GetMethod("OpenBaseKey", new Type[] { hiveType, viewType });
        MethodInfo openKey = keyType.GetMethod("OpenSubKey", new Type[] { typeof(string) });
        MethodInfo getValue = keyType.GetMethod("GetValue", new Type[] { typeof(string) });
        if (openBase == null || openKey == null || getValue == null)
            return false;
        foreach (string view in new string[] { "Registry64", "Registry32" })
        {
            foreach (string hive in new string[] { "LocalMachine", "CurrentUser" })
            {
                try
                {
                    using (IDisposable root = (IDisposable)openBase.Invoke(null, new object[] { Enum.Parse(hiveType, hive), Enum.Parse(viewType, view) }))
                    {
                        foreach (string name in new string[] { @"SOFTWARE\LLVM\LLVM", @"SOFTWARE\LLVM" })
                        {
                            using (IDisposable key = (IDisposable)openKey.Invoke(root, new object[] { name }))
                            {
                                if (key == null)
                                    continue;
                                foreach (string value in new string[] { "", "InstallDir", "InstallLocation" })
                                {
                                    string directory = getValue.Invoke(key, new object[] { value }) as string;
                                    if (!String.IsNullOrWhiteSpace(directory) && TryBin(Path.Combine(directory, "bin")))
                                        return true;
                                }
                            }
                        }
                    }
                }
                catch (System.Security.SecurityException) { }
                catch (UnauthorizedAccessException) { }
                catch (IOException) { }
                catch (TargetInvocationException) { }
            }
        }
        return false;
    }

    private static string ReadSmallFile(string path)
    {
        try
        {
            if (!File.Exists(path) || new FileInfo(path).Length > 1048576)
                return null;
            return File.ReadAllText(path).TrimEnd('\r', '\n');
        }
        catch (IOException) { return null; }
        catch (UnauthorizedAccessException) { return null; }
        catch (System.Security.SecurityException) { return null; }
    }

    private bool TryCache(string directory)
    {
        foreach (string relative in new string[] { "CMakeCache.txt", Path.Combine("build", "CMakeCache.txt"), Path.Combine("build", "vs2022-win32", "CMakeCache.txt") })
        {
            string content = ReadSmallFile(Path.Combine(directory, relative));
            if (content == null)
                continue;
            foreach (string line in content.Split('\n'))
            {
                const string prefix = "NFS_BYTE_DIVISION_COMPILER:FILEPATH=";
                if (line.StartsWith(prefix, StringComparison.Ordinal) && TryCompiler(line.Substring(prefix.Length).TrimEnd('\r')))
                    return true;
            }
        }
        return false;
    }

    private static string[] Directories(string path)
    {
        try
        {
            if ((File.GetAttributes(path) & FileAttributes.ReparsePoint) != 0)
                return new string[0];
            return Directory.GetDirectories(path);
        }
        catch (IOException) { return new string[0]; }
        catch (UnauthorizedAccessException) { return new string[0]; }
        catch (System.Security.SecurityException) { return new string[0]; }
    }

    private bool TryNearbyCaches()
    {
        if (TryCache(ProjectDirectory))
            return true;
        DirectoryInfo parent = Directory.GetParent(ProjectDirectory.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar));
        for (int depth = 0; depth < 2 && parent != null; ++depth, parent = parent.Parent)
        {
            foreach (string directory in Directories(parent.FullName))
                if (Path.GetFileName(directory).StartsWith("nfshp2", StringComparison.OrdinalIgnoreCase) && TryCache(directory))
                    return true;
        }
        return false;
    }

    private bool TryDocuments()
    {
        var roots = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        if (DocumentsDirectories != null)
        {
            foreach (ITaskItem item in DocumentsDirectories)
                roots.Add(item.ItemSpec);
        }
        else
        {
            roots.Add(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments));
            roots.Add(Environment.GetFolderPath(Environment.SpecialFolder.CommonDocuments));
            string profile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            if (!String.IsNullOrEmpty(profile))
                roots.Add(Path.Combine(profile, "Documents"));
            foreach (string variable in new string[] { "OneDrive", "OneDriveConsumer", "OneDriveCommercial" })
            {
                string directory = Environment.GetEnvironmentVariable(variable);
                if (!String.IsNullOrEmpty(directory))
                    roots.Add(Path.Combine(directory, "Documents"));
            }
        }
        var queue = new Queue<KeyValuePair<string, int>>();
        foreach (string root in roots)
            if (!String.IsNullOrEmpty(root) && Directory.Exists(root))
                queue.Enqueue(new KeyValuePair<string, int>(root, 0));
        int visited = 0;
        while (queue.Count != 0 && visited++ < 8192)
        {
            KeyValuePair<string, int> current = queue.Dequeue();
            try
            {
                if ((File.GetAttributes(current.Key) & FileAttributes.ReparsePoint) != 0)
                    continue;
                if (TryBin(Path.Combine(current.Key, "tools", "llvm", "compiler", "bin")))
                    return true;
                if (current.Value < 4)
                    foreach (string child in Directories(current.Key))
                        queue.Enqueue(new KeyValuePair<string, int>(child, current.Value + 1));
            }
            catch (IOException) { }
            catch (UnauthorizedAccessException) { }
            catch (System.Security.SecurityException) { }
        }
        return false;
    }

    private bool FindCompiler()
    {
        if (TryBin(Path.Combine(ProjectDirectory, "tools", "llvm", "bin")) ||
            TryBin(Path.Combine(ProjectDirectory, "tools", "llvm", "compiler", "bin")))
            return true;
        string previous = ReadSmallFile(Path.Combine(RecordDirectory, "compiler.path"));
        if (TryCompiler(previous) || TryNearbyCaches())
            return true;
        if (!String.IsNullOrEmpty(VisualCppDirectory) &&
            (TryBin(Path.Combine(VisualCppDirectory, "Tools", "Llvm", "x64", "bin")) ||
             TryBin(Path.Combine(VisualCppDirectory, "Tools", "Llvm", "bin"))))
            return true;
        foreach (Environment.SpecialFolder folder in new Environment.SpecialFolder[] { Environment.SpecialFolder.ProgramFiles, Environment.SpecialFolder.ProgramFilesX86 })
        {
            string directory = Environment.GetFolderPath(folder);
            if (!String.IsNullOrEmpty(directory) && TryBin(Path.Combine(directory, "LLVM", "bin")))
                return true;
        }
        if (TryRegistry())
            return true;
        string searchPath = SearchPath ?? Environment.GetEnvironmentVariable("PATH") ?? "";
        foreach (string directory in searchPath.Split(Path.PathSeparator))
            if (!String.IsNullOrWhiteSpace(directory) && TryBin(directory.Trim().Trim('"')))
                return true;
        if (TryDocuments())
            return true;
        return false;
    }

    public override bool Execute()
    {
        try
        {
            ProjectDirectory = Path.GetFullPath(ProjectDirectory);
            if (!FindCompiler())
            {
                Log.LogError("clang-cl.exe was not found in the project, Visual Studio, LLVM installations, PATH, previous project builds, or portable tool folders. The math sources require this compiler; MSVC cannot replace their native 80-bit arithmetic.");
                return false;
            }
            CompilerDirectory = Path.GetDirectoryName(executable) + Path.DirectorySeparatorChar;
            CompilerFile = new ITaskItem[] { new TaskItem(executable) };
            var file = new FileInfo(executable);
            string identity = executable + "\n" + file.Length + "\n" + file.LastWriteTimeUtc.Ticks;
            using (SHA256 hash = SHA256.Create())
                CompilerSignature = BitConverter.ToString(hash.ComputeHash(Encoding.UTF8.GetBytes(identity))).Replace("-", "").ToLowerInvariant();
            CompilerChanged = ReadSmallFile(Path.Combine(RecordDirectory, "compiler.signature")) != CompilerSignature;
            Log.LogMessage(MessageImportance.Low, "C++ compiler for extended-precision sources: {0}", executable);
            return true;
        }
        catch (Exception exception)
        {
            Log.LogError("Cannot locate the C++ compiler: {0}", exception.Message);
            return false;
        }
    }
}
