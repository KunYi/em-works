// *******************************************************************************************
//
//  makexip.js
//
//  Usage:    cscript //nologo makexip.js <-bib | -reg> <regiontable> <file to process>
//
//  Copyright Microsoft Corporation, All Rights Reserved.
//
// *******************************************************************************************
//


var shell      = WScript.CreateObject("WScript.Shell");
var fileSystem = WScript.CreateObject("Scripting.FileSystemObject");
var environmentVariables = shell.Environment("Process");
var argError = "Incorrect arguments\nExample:\ncscript //nologo MakeBinfsBib.js <-bib> <file to process>";

var table = new Array();
table[0] = new Array();
table[0][0] = "<DONTCOMPRESS>nk.exe";
table[0][1] = "XIPKERNEL";
table[1] = new Array();
table[1][0] = "<DONTCOMPRESS>kd.dll";
table[1][1] = "XIPKERNEL";
table[2] = new Array();
table[2][0] = "<DONTCOMPRESS>hd.dll";
table[2][1] = "XIPKERNEL";
table[3] = new Array();
table[3][0] = "<DONTCOMPRESS>osaxst0.dll";
table[3][1] = "XIPKERNEL";
table[4] = new Array();
table[4][0] = "<DONTCOMPRESS>osaxst1.dll";
table[4][1] = "XIPKERNEL";
table[5] = new Array();
table[5][0] = "<DONTCOMPRESS>coredll.dll";
table[5][1] = "XIPKERNEL";
table[6] = new Array();
table[6][0] = "<DONTCOMPRESS>filesys.exe";
table[6][1] = "XIPKERNEL";
table[7] = new Array();
table[7][0] = "<DONTCOMPRESS>fsdmgr.dll";
table[7][1] = "XIPKERNEL";
table[8] = new Array();
table[8][0] = "<DONTCOMPRESS>mspart.dll";
table[8][1] = "XIPKERNEL";
table[9] = new Array();
table[9][0] = "<DONTCOMPRESS>binfs.dll";
table[9][1] = "XIPKERNEL";
table[10] = new Array();
table[10][0] = "<DONTCOMPRESS>ceddk.dll";
table[10][1] = "XIPKERNEL";
table[11] = new Array();
table[11][0] = "initobj.dat";
table[11][1] = "XIPKERNEL";
table[12] = new Array();
table[12][0] = "default.fdf";
table[12][1] = "XIPKERNEL";
table[13] = new Array();
table[13][0] = "initdb.ini";
table[13][1] = "XIPKERNEL";
table[14] = new Array();
table[14][0] = "boot.hv";
table[14][1] = "XIPKERNEL";
table[15] = new Array();
table[15][0] = "default.hv";
table[15][1] = "XIPKERNEL";
table[16] = new Array();
table[16][0] = "user.hv";
table[16][1] = "XIPKERNEL";
table[17] = new Array();
table[17][0] = "<DONTCOMPRESS>device.exe";
table[17][1] = "XIPKERNEL";
table[18] = new Array();
table[18][0] = "<DONTCOMPRESS>devmgr.dll";
table[18][1] = "XIPKERNEL";
table[19] = new Array();
table[19][0] = "<DONTCOMPRESS>filesys.dll";
table[19][1] = "XIPKERNEL";
table[20] = new Array();
table[20][0] = "<DONTCOMPRESS>romfsd.dll";
table[20][1] = "XIPKERNEL";
table[21] = new Array();
table[21][0] = "<DONTCOMPRESS>fpcrt.dll";
table[21][1] = "XIPKERNEL";
table[22] = new Array();
table[22][0] = "<DONTCOMPRESS>k.fpcrt.dll";
table[22][1] = "XIPKERNEL";
table[23] = new Array();
table[23][0] = "<DONTCOMPRESS>device.dll";
table[23][1] = "XIPKERNEL";
table[24] = new Array();
table[24][0] = "<DONTCOMPRESS>kernel.dll";
table[24][1] = "XIPKERNEL";
table[25] = new Array();
table[25][0] = "<DONTCOMPRESS>oalioctl.dll";
table[25][1] = "XIPKERNEL";
table[26] = new Array();
table[26][0] = "<DONTCOMPRESS>k.coredll.dll";
table[26][1] = "XIPKERNEL";
table[27] = new Array();
table[27][0] = "<DONTCOMPRESS>servicesEnum.dll";
table[27][1] = "XIPKERNEL";
table[28] = new Array();
table[28][0] = "<DONTCOMPRESS>servicesd.exe";
table[28][1] = "XIPKERNEL";
table[29] = new Array();
table[29][0] = "<DONTCOMPRESS>services.exe";
table[29][1] = "XIPKERNEL";
table[30] = new Array();
table[30][0] = "<DONTCOMPRESS>servicesstart.exe";
table[30][1] = "XIPKERNEL";
table[31] = new Array();
table[31][0] = "<DONTCOMPRESS>flashmdd.dll";
table[31][1] = "XIPKERNEL";
table[32] = new Array();
table[32][0] = "<DONTCOMPRESS>flashpart.dll";
table[32][1] = "XIPKERNEL";
table[33] = new Array();
table[33][0] = "<DONTCOMPRESS>pm.dll";
table[33][1] = "XIPKERNEL";
table[34] = new Array();
table[34][0] = "<DONTCOMPRESS>kitl.dll";
table[34][1] = "XIPKERNEL";
table[35] = new Array();
table[35][0] = "<DONTCOMPRESS>regenum.dll";
table[35][1] = "XIPKERNEL";
table[36] = new Array();
table[36][0] = "<DONTCOMPRESS>busenum.dll";
table[36][1] = "XIPKERNEL";

var currentTag = null;
var tagStack = [];

var NoMode = 0;
var BibMode = 1;
var mode = NoMode;
var bibSection = ""; // defines where we are in the ce.bib file (FILES section, or MODULES section, etc)
var binfsEnabled = EnvVarSet("BSP_SUPPORT_DEMAND_PAGING");

var bibTempFile = "bibfiletmp.tmp";
var bibTempOutput;


try
{
    if(!binfsEnabled)
    {
        WScript.echo("BINFS not enabled, skip MakeBinfsBib.js.");
        WScript.Quit(0);
    }

    // process arguments
    if (WScript.Arguments.Length != 2)
        throw argError;

    if (WScript.Arguments(0) == "-bib")
    {
        bibTempOutput = fileSystem.CreateTextFile(bibTempFile);
        mode = BibMode;
    }
    else
        throw "-bib must be the first argument";

    tagStack.push(null);

    var file = WScript.Arguments(1);
    var input = fileSystem.OpenTextFile(file);

    ProcessFile(input);

    input.close();

    // finalize and cleanup
    if (mode == BibMode)
    {
        bibTempOutput.close();
        fileSystem.CopyFile(bibTempFile, file, true);
        fileSystem.DeleteFile(bibTempFile);
    }
}
catch(err)
{
    if (err != argError)
    {
        WScript.echo("MakeBinfsBib.js: "+err);
        throw err;
    }
    else
    {
        WScript.echo(err);
    }
    WScript.Quit(-1);
}

// takes the input line and processes for BibMode.
// this is the guts of the Bib processing
function ProcessBibLine(line)
{
    //    TOKENS:
    //      1                                                    2                  3             4      5   6  7
    //    msmqd.dll    $(_FLATRELEASEDIR)\msmqd.dll                       NK   FILE   SHX
    //       OR
    //      1                                                    2                  3             4      5
    //    msmqd.dll    $(_FLATRELEASEDIR)\msmqd.dll                       NK   SHX

    var tokens = line.match(/\s*(\S+)\s+\S+\\(\S+)(\s+)NK(\s*)(\S*)(\s*)(\S*)/);

    if (tokens != null)
    {
        var filename = tokens[1];   // first file name in the line
        var targetfile = tokens[1];   // target file name in the line
        var sourcefile = tokens[2];   // source file name in the line

        // search for filename first
        var value = GetTableValue(filename);

        // if the first file wasn't found, try the file in the second column
        if (value == null)
        {
            filename = tokens[2];
            value = GetTableValue(filename);
        }

        // if no files were found, use the tag
        if (value == null)
            value = GetTableValue(currentTag);

        if (value != null)
        {
            // if we didn't match all the tags, then the FILE/MODULE wasn't included, so let's
            // bump them up
            if (tokens[7] == "")
            {
                tokens[7] = tokens[5];
                tokens[6] = "  ";
                tokens[5] = "";
            }

            if (CheckOptions(value[0], "DONTCOMPRESS"))
                tokens[5] = " MODULE ";
            else if (CheckOptions(value[0],"COMPRESS"))
            {
                if (tokens[7].search(/C/i) == -1)
                {
                    tokens[7] = tokens[7] + "C";
                }
            }

            line = line.replace(/(\s+)NK(\s*)(\S*)(\s*)(\S*)/, tokens[3] + value[1] + tokens[4] + tokens[5] + tokens[6] + tokens[7]);
        }
    }
    else if (line.search(/^FILES$/) != -1)
    {
//        WScript.Echo("FILES SECTION");
        bibSection = "FILES";
    }
    else if (line.search(/^MODULES$/) != -1)
    {
//        WScript.Echo("MODULES SECTION");
        bibSection = "MODULES";
    }

    if (value != null && value[1].toUpperCase() == "IGNORE")
    {
        // if we see the 'IGNORE' value, then we will not write to the bib file
    }
    else if (value != null && CheckOptions(value[0], "FILEREPLACE") && sourcefile == targetfile)
    {
        // if we see the 'FILEREPLACE' value, and the source file name is same as the target file name, 
        // this file will be replaced from platform.bib, then we will not write to the bib file
    }
    else
    {
        bibTempOutput.WriteLine(line);
    }
}

// Simple Function to check if a IMGflag type env variable has been set
function EnvVarSet(env)
{
    return environmentVariables(env) != "";
}

// check to see if the option exists in the list of options
function CheckOptions(list, option)
{
    var result = false;
    for (var index = 0; index < list.length; index++)
    {
        if (option.toUpperCase() == list[index].toUpperCase())
        {
            result = true;
            break;
        }
    }

    return result;
}

// given a tag, return the region and the options
// value[0] is an array of options in <> (array of strings)
// value[1] is the region (string)
function GetTableValue(tag)
{
    var found = false;
    var value = [];

    for (var index = table.length - 1; index >= 0; index--)
    {
        var tableTag = "";
        var options = [];

        var tableTagMatch = table[index][0].match(/<(.*)>(.*)/);
        if (tableTagMatch != null)
        {
            options = tableTagMatch[1].split(/\s*,\s*/);
            tableTag = tableTagMatch[2];
        }
        else
        {
            tableTag = table[index][0];
        }

        if (tableTag != null && tag != null &&
            tableTag.toLowerCase() == tag.toLowerCase())
        {
            // OPTION Tag
            value[0] = options;

            // Actual Value
            value[1] = table[index][1];
            found = true;
            break;
        }
    }

    if (found)
        return value;
    else
        return null;
}


// main function that processes the input file.
// this function calls the appropriate function to handle
// each line of the input file based on the mode the script
// was invoked
function ProcessFile(input)
{
    while(!input.AtEndOfStream)
    {
        var line = input.ReadLine();

        if (mode == BibMode)
        {
            ProcessBibLine(line);
        }
    }
}

