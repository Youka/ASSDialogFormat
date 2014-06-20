/*
Project: ASSDialogFormat
File: main.c

Copyright (c) 2014, Christoph "Youka" Spanknebel

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

    The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// Replace string in string and create new one
char* str_replace(const char* original, const char* find, const char* replacement, char free_original){
    int find_len = strlen(find), found_count = 0;
    char* result;
    const char* poriginal = original, *found;
    while(found = strstr(poriginal, find)){
        found_count++;
        poriginal = found + find_len;
    }
    result = malloc(strlen(original) + found_count * (strlen(replacement) - find_len) + 1);
    if(result){
        result[0] = '\0';
        poriginal = original;
        while(found = strstr(poriginal, find)){
            strncat(result, poriginal, found-poriginal);
            strcat(result, replacement);
            poriginal = found + find_len;
        }
        strcat(result, poriginal);
    }
    if(free_original)
        free((void*)original);
    return result;
}

// Converts time units to ASS (Advanced Substation Alpha) timestamp
void ass_timestamp(char* buf, int h, int m, int s, int ms10){
    buf[0] = h % 10 + '0';
    buf[1] = ':';
    buf[2] = m / 10 + '0';
    buf[3] = m % 10 + '0';
    buf[4] = ':';
    buf[5] = s / 10 + '0';
    buf[6] = s % 10 + '0';
    buf[7] = '.';
    buf[8] = ms10 / 10 + '0';
    buf[9] = ms10 % 10 + '0';
    buf[10] = '\0';
}

// Program entry
int main(int argc, const char** argv){
    // Declarations & default definitions
    const char* ifilename = 0, *ofilename = 0, *format = "!start-!end\\t!actor\\t!text\\n", *pline, *pline2;
    FILE* ifile = stdin, *ofile = stdout;
    double old_fps = 0.0, new_fps = 0.0, fps_mul;
    int i, len, layer, start_h, start_m, start_s, end_h, end_m, end_s;
    long start_ms, end_ms;
    char line[2048], style[64], actor[64], effect[256], text[1024], *format_compiled;
    // Program description
    if(argc < 2){
        puts(
"This program converts an ASS file to a specific formatted file with dialog informations.\n\
If no filename is passed, stdin is read and to stdout is written.\n\
\n\
Arguments:\n\
<ass_filename>\tInput ASS (Advanced Substation Alpha) file for conversion.\n\
-o <out_filename>\tOutput file as conversion result.\n\
-ofps <fps_number>\tOld FPS (frames-per-second) as conversion base.\n\
-nfps <fps_number>\tNew FPS as conversion result.\n\
-f <format_string>\tFormat description for output. <\"!start-!end\\t!actor\\t!text\\n\">\n\
\n\
Format patterns:\n\
!layer\n\
!start\n\
!end\n\
!style\n\
!actor\n\
!effect\n\
!text"
);
        return 0;
    }
    // Read program arguments
    for(i = 1; i < argc; ++i){
        if(strcmp(argv[i], "-f") == 0){
            if(i+1 == argc){
                puts("Expected a format string after flag -f!");
                return 1;
            }
            format = argv[i+1];
            i++;
        }else if(strcmp(argv[i], "-nfps") == 0){
            if(i+1 == argc){
                puts("Expected a fps number after flag -nfps!");
                return 1;
            }
            new_fps = strtod(argv[i+1], 0);
            if(new_fps <= 0.0){
                puts("Expected a valid number (>0) for -nfps!");
                return 1;
            }
            i++;
        }else if(strcmp(argv[i], "-ofps") == 0){
            if(i+1 == argc){
                puts("Expected a fps number after flag -ofps!");
                return 1;
            }
            old_fps = strtod(argv[i+1], 0);
            if(old_fps <= 0.0){
                puts("Expected a valid number (>0) for -ofps!");
                return 1;
            }
            i++;
        }else if(strcmp(argv[i], "-o") == 0){
            if(i+1 == argc){
                puts("Expected a filename after flag -o!");
                return 1;
            }
            ofilename = argv[i+1];
            i++;
        }else
            ifilename = argv[i];
    }
    // Calculate fps conversion
    fps_mul = old_fps <= 0.0 || new_fps <= 0.0 ? 1.0 : old_fps / new_fps;
    // Open files
    if(ifilename){
        ifile = fopen(ifilename, "r");
        if(!ifile){
            printf("Couldn't open input file \"%s\"!", ifilename);
            return 1;
        }
    }
    if(ofilename){
        ofile = fopen(ofilename, "w");
        if(!ofile){
            printf("Couldn't open output file \"%s\"!", ofilename);
            return 1;
        }
    }
    // Compile format string
    format_compiled = str_replace(str_replace(format, "\\t", "\t", 0), "\\n", "\n", 1);
    // Iterate through input file lines
    while(fgets(line, sizeof(line), ifile))
        // Is dialog line and can read leading numbers?
        if(sscanf(line, "Dialogue: %d,%d:%d:%d.%d,%d:%d:%d.%d,", &layer, &start_h, &start_m, &start_s, &start_ms, &end_h, &end_m, &end_s, &end_ms) == 9){
            // Overjump previous content
            pline = strchr(strchr(strchr(line, ',')+1, ',')+1, ',')+1;
            // Read style
            pline2 = strchr(pline, ',');
            if(!pline2)
                continue;
            len = MIN(sizeof(style)-1, pline2-pline);
            memcpy(style, pline, len);
            style[len] = '\0';
            pline = pline2 + 1;
            // Read actor
            pline2 = strchr(pline, ',');
            if(!pline2)
                continue;
            len = MIN(sizeof(actor)-1, pline2-pline);
            memcpy(actor, pline, len);
            actor[len] = '\0';
            pline = pline2 + 1;
            // Overjump margins
            i = 0;
            while((pline = strchr(pline, ',')+1) != (const char*)1 && ++i < 3);
            if(pline == (const char*)1)
                continue;
            // Read effect
            pline2 = strchr(pline, ',');
            if(!pline2)
                continue;
            len = MIN(sizeof(effect)-1, pline2-pline);
            memcpy(effect, pline, len);
            effect[len] = '\0';
            pline = pline2 + 1;
            // Read text
            pline2 = strchr(pline, '\n');
            if(pline2){
                len = MIN(sizeof(text)-1, pline2-pline);
                memcpy(text, pline, len);
                text[len] = '\0';
            }else{
                strncpy(text, pline, sizeof(text));
                text[sizeof(text)-1] = '\0';
            }
            // Convert times
            if(fps_mul != 1.0){
                start_ms = (start_ms * 10 + start_s * 1000 + start_m * 60000 + start_h * 3600000) * fps_mul;
                start_h = start_ms / 3600000;
                start_ms %= 3600000;
                start_m = start_ms / 60000;
                start_ms %= 60000;
                start_s = start_ms / 1000;
                start_ms = start_ms % 1000 / 10;
                end_ms = (end_ms * 10 + end_s * 1000 + end_m * 60000 + end_h * 3600000) * fps_mul;
                end_h = end_ms / 3600000;
                end_ms %= 3600000;
                end_m = end_ms / 60000;
                end_ms %= 60000;
                end_s = end_ms / 1000;
                end_ms = end_ms % 1000 / 10;
            }
            // Write formatted dialog to output
            itoa(layer, line, 10);
            pline = str_replace(format_compiled, "!layer", line, 0);
            ass_timestamp(line, start_h, start_m, start_s, start_ms);
            pline = str_replace(pline, "!start", line, 1);
            ass_timestamp(line, end_h, end_m, end_s, end_ms);
            pline = str_replace(pline, "!end", line, 1);
            pline = str_replace(pline, "!style", style, 1);
            pline = str_replace(pline, "!actor", actor, 1);
            pline = str_replace(pline, "!effect", effect, 1);
            pline = str_replace(pline, "!text", text, 1);
            fputs(pline, ofile);
            free((void*)pline);
        }
    // Free memory and close open file handles
    free(format_compiled);
    if(ifile != stdin)
        fclose(ifile);
    if(ofile != stdout)
        fclose(ofile);
    // End program with success
    return 0;
}
