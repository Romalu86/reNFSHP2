using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

public sealed class NfsSelectCompileInputs : Task
{
    [Required] public ITaskItem[] Sources { get; set; }
    [Required] public string CompilerPhase { get; set; }
    [Required] public string ProjectDirectory { get; set; }
    public string SelectedFiles { get; set; }
    [Output] public ITaskItem[] ActiveSources { get; set; }
    [Output] public bool ClangRequested { get; set; }
    [Output] public bool CpuSelected { get; set; }

    public override bool Execute()
    {
        try
        {
            if (CompilerPhase != "MSVC" && CompilerPhase != "Clang")
                throw new InvalidDataException("Unknown compiler phase: " + CompilerPhase);
            var paths = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            bool allFiles = String.IsNullOrWhiteSpace(SelectedFiles);
            if (!allFiles)
            {
                foreach (string item in SelectedFiles.Split(';'))
                {
                    string selected = item.Trim().Trim('"');
                    if (selected.Length == 0)
                        continue;
                    paths.Add(Path.GetFullPath(Path.Combine(ProjectDirectory, selected)));
                    if (selected.IndexOfAny(new char[] { '/', '\\' }) < 0)
                        names.Add(selected);
                }
            }
            var active = new List<ITaskItem>();
            foreach (ITaskItem source in Sources)
            {
                string fullPath = source.GetMetadata("FullPath");
                if (String.IsNullOrEmpty(fullPath))
                    fullPath = Path.GetFullPath(Path.Combine(ProjectDirectory, source.ItemSpec));
                bool selected = allFiles || paths.Contains(fullPath) || names.Contains(Path.GetFileName(fullPath));
                if (!selected)
                    continue;
                string compiler = source.GetMetadata("NfsCompiler");
                if (compiler != "MSVC" && compiler != "Clang")
                    throw new InvalidDataException("No compiler assigned to " + source.ItemSpec);
                if (compiler == "Clang")
                    ClangRequested = true;
                if (compiler != CompilerPhase)
                    continue;
                active.Add(new TaskItem(source));
                if (source.GetMetadata("NfsCpuProbe") == "true")
                    CpuSelected = true;
            }
            ActiveSources = active.ToArray();
            return true;
        }
        catch (Exception exception)
        {
            Log.LogError("Cannot select compiler inputs: {0}", exception.Message);
            return false;
        }
    }
}
