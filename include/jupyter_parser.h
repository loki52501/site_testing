#ifndef JUPYTER_PARSER_H
#define JUPYTER_PARSER_H

#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

struct NotebookCell {
    std::string cellType;        // "code", "markdown", "raw"
    std::string source;          // Combined source lines
    std::vector<std::string> outputs; // HTML-rendered outputs
    std::string language;        // "python", "r", "julia"
    int executionCount;          // Execution count for code cells
};

class JupyterParser {
public:
    JupyterParser();

    // Main conversion function
    std::string convertToHTML(const std::string& ipynbContent,
                               const std::string& notebookPath,
                               std::vector<std::string>& extractedImages);

private:
    // Parse .ipynb JSON structure
    std::vector<NotebookCell> parseCells(const json& notebook);

    // Render individual cell types
    std::string renderCodeCell(const NotebookCell& cell, int cellNumber);
    std::string renderMarkdownCell(const NotebookCell& cell);
    std::string renderOutputs(const std::vector<std::string>& outputs, int executionCount,
                               const std::string& notebookPath, int cellNumber,
                               std::vector<std::string>& extractedImages);

    // Output parsers
    std::string parseOutput(const json& output, const std::string& notebookPath,
                            int cellNumber, int outputNumber,
                            std::vector<std::string>& extractedImages);
    std::string parseStreamOutput(const json& output);
    std::string parseDisplayData(const json& output, const std::string& notebookPath,
                                  int cellNumber, int outputNumber,
                                  std::vector<std::string>& extractedImages);
    std::string parseExecuteResult(const json& output, const std::string& notebookPath,
                                    int cellNumber, int outputNumber,
                                    std::vector<std::string>& extractedImages);
    std::string parseErrorOutput(const json& output);

    // Image handling
    std::string extractBase64Image(const std::string& base64Data,
                                    const std::string& mimeType,
                                    const std::string& notebookPath,
                                    int cellNumber, int outputNumber,
                                    std::vector<std::string>& extractedImages);
    std::string saveImage(const std::vector<unsigned char>& imageData,
                          const std::string& outputPath);

    // Helper functions
    std::string escapeHTML(const std::string& str);
    std::string joinLines(const json& lines);
    std::string detectLanguage(const json& metadata);
    std::string getPrismLanguageClass(const std::string& language);
    std::string base64Decode(const std::string& encoded);
};

#endif
