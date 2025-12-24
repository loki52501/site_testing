#include "../include/jupyter_parser.h"
#include "../include/markdown_parser.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <regex>

namespace fs = std::filesystem;

JupyterParser::JupyterParser() {}

std::string JupyterParser::convertToHTML(const std::string& ipynbContent,
                                          const std::string& notebookPath,
                                          std::vector<std::string>& extractedImages) {
    try {
        // Parse the JSON content
        json notebook = json::parse(ipynbContent);

        // Parse all cells
        std::vector<NotebookCell> cells = parseCells(notebook);

        // Detect the programming language from metadata
        std::string language = "python";
        if (notebook.contains("metadata")) {
            language = detectLanguage(notebook["metadata"]);
        }

        // Build HTML output
        std::stringstream html;
        html << "<div class=\"jupyter-notebook\">\n";

        int cellNumber = 0;
        for (const auto& cell : cells) {
            cellNumber++;

            if (cell.cellType == "markdown") {
                html << renderMarkdownCell(cell);
            } else if (cell.cellType == "code") {
                html << renderCodeCell(cell, cellNumber);

                // Process outputs if they exist
                if (!cell.outputs.empty()) {
                    html << renderOutputs(cell.outputs, cell.executionCount, notebookPath, cellNumber, extractedImages);
                }
            }
            // Skip raw cells
        }

        html << "</div>\n";
        return html.str();

    } catch (const json::exception& e) {
        return "<div class=\"error\">Error parsing notebook: " + std::string(e.what()) + "</div>";
    }
}

std::vector<NotebookCell> JupyterParser::parseCells(const json& notebook) {
    std::vector<NotebookCell> cells;

    if (!notebook.contains("cells") || !notebook["cells"].is_array()) {
        return cells;
    }

    std::string notebookLanguage = "python";
    if (notebook.contains("metadata")) {
        notebookLanguage = detectLanguage(notebook["metadata"]);
    }

    for (const auto& cellJson : notebook["cells"]) {
        NotebookCell cell;

        // Get cell type
        if (cellJson.contains("cell_type")) {
            cell.cellType = cellJson["cell_type"];
        }

        // Get source content
        if (cellJson.contains("source")) {
            cell.source = joinLines(cellJson["source"]);
        }

        // Get language (default to notebook language)
        cell.language = notebookLanguage;

        // Get execution count for code cells
        cell.executionCount = 0;
        if (cell.cellType == "code" && cellJson.contains("execution_count")) {
            if (!cellJson["execution_count"].is_null()) {
                cell.executionCount = cellJson["execution_count"];
            }
        }

        // Parse outputs for code cells
        if (cell.cellType == "code" && cellJson.contains("outputs")) {
            // We'll store raw output JSON as strings temporarily
            // They will be parsed when rendering
            for (const auto& output : cellJson["outputs"]) {
                cell.outputs.push_back(output.dump());
            }
        }

        cells.push_back(cell);
    }

    return cells;
}

std::string JupyterParser::renderMarkdownCell(const NotebookCell& cell) {
    // Use the existing MarkdownParser to convert markdown to HTML
    MarkdownParser mdParser;
    std::string htmlContent = mdParser.convertToHTML(cell.source);

    std::stringstream html;
    html << "  <div class=\"nb-cell nb-markdown-cell\">\n";
    html << "    " << htmlContent << "\n";
    html << "  </div>\n";

    return html.str();
}

std::string JupyterParser::renderCodeCell(const NotebookCell& cell, int cellNumber) {
    std::stringstream html;
    std::string languageClass = getPrismLanguageClass(cell.language);

    html << "  <div class=\"nb-cell nb-code-cell\">\n";
    html << "    <div class=\"nb-input\">\n";

    // Add input prompt with execution count
    if (cell.executionCount > 0) {
        html << "      <div class=\"nb-input-prompt\">In [" << cell.executionCount << "]:</div>\n";
    } else {
        html << "      <div class=\"nb-input-prompt\">In [ ]:</div>\n";
    }

    // Add code with Prism.js syntax highlighting
    html << "      <pre><code class=\"" << languageClass << "\">";
    html << escapeHTML(cell.source);
    html << "</code></pre>\n";
    html << "    </div>\n";

    return html.str();
}

std::string JupyterParser::renderOutputs(const std::vector<std::string>& outputs, int executionCount,
                                          const std::string& notebookPath, int cellNumber,
                                          std::vector<std::string>& extractedImages) {
    if (outputs.empty()) {
        return "";
    }

    std::stringstream html;

    int outputNumber = 0;
    for (const auto& outputStr : outputs) {
        outputNumber++;

        try {
            json output = json::parse(outputStr);
            std::string outputType = output.value("output_type", "");

            if (outputType == "stream") {
                html << "    <div class=\"nb-output\">\n";
                if (executionCount > 0) {
                    html << "      <div class=\"nb-output-prompt\"></div>\n";
                }
                html << parseStreamOutput(output);
                html << "    </div>\n";
            } else if (outputType == "display_data" || outputType == "execute_result") {
                html << "    <div class=\"nb-output\">\n";

                // Add output prompt for execute_result
                if (outputType == "execute_result" && executionCount > 0) {
                    html << "      <div class=\"nb-output-prompt\">Out[" << executionCount << "]:</div>\n";
                }

                // Parse the output data
                if (outputType == "display_data") {
                    html << parseDisplayData(output, notebookPath, cellNumber, outputNumber, extractedImages);
                } else {
                    html << parseExecuteResult(output, notebookPath, cellNumber, outputNumber, extractedImages);
                }

                html << "    </div>\n";
            } else if (outputType == "error") {
                html << "    <div class=\"nb-output\">\n";
                html << parseErrorOutput(output);
                html << "    </div>\n";
            }

        } catch (const json::exception& e) {
            html << "    <div class=\"nb-output-error\">Error parsing output: " << e.what() << "</div>\n";
        }
    }

    html << "  </div>\n";  // Close nb-code-cell
    return html.str();
}

std::string JupyterParser::parseStreamOutput(const json& output) {
    std::stringstream html;

    if (output.contains("text")) {
        std::string text = joinLines(output["text"]);
        html << "      <pre>" << escapeHTML(text) << "</pre>\n";
    }

    return html.str();
}

std::string JupyterParser::parseDisplayData(const json& output,
                                             const std::string& notebookPath,
                                             int cellNumber, int outputNumber,
                                             std::vector<std::string>& extractedImages) {
    std::stringstream html;

    if (!output.contains("data")) {
        return html.str();
    }

    const json& data = output["data"];

    // Check for images first (PNG, SVG, JPEG)
    if (data.contains("image/png")) {
        std::string imgHtml = extractBase64Image(data["image/png"], "image/png",
                                                  notebookPath, cellNumber, outputNumber,
                                                  extractedImages);
        html << "      " << imgHtml << "\n";
    } else if (data.contains("image/svg+xml")) {
        // For SVG, we can embed it directly
        std::string svgContent = joinLines(data["image/svg+xml"]);
        html << "      " << svgContent << "\n";
    } else if (data.contains("image/jpeg")) {
        std::string imgHtml = extractBase64Image(data["image/jpeg"], "image/jpeg",
                                                  notebookPath, cellNumber, outputNumber,
                                                  extractedImages);
        html << "      " << imgHtml << "\n";
    }
    // Check for HTML output
    else if (data.contains("text/html")) {
        std::string htmlContent = joinLines(data["text/html"]);
        html << "      <div class=\"nb-html-output\">" << htmlContent << "</div>\n";
    }
    // Check for plain text
    else if (data.contains("text/plain")) {
        std::string text = joinLines(data["text/plain"]);
        html << "      <pre>" << escapeHTML(text) << "</pre>\n";
    }

    return html.str();
}

std::string JupyterParser::parseExecuteResult(const json& output,
                                                const std::string& notebookPath,
                                                int cellNumber, int outputNumber,
                                                std::vector<std::string>& extractedImages) {
    // Execute result has the same structure as display_data
    return parseDisplayData(output, notebookPath, cellNumber, outputNumber, extractedImages);
}

std::string JupyterParser::parseErrorOutput(const json& output) {
    std::stringstream html;

    if (output.contains("ename") && output.contains("evalue")) {
        std::string ename = output["ename"];
        std::string evalue = output["evalue"];

        html << "      <div class=\"nb-output-error\">";
        html << "<strong>" << escapeHTML(ename) << ":</strong> " << escapeHTML(evalue);

        // Add traceback if available
        if (output.contains("traceback")) {
            html << "\n\n";
            for (const auto& line : output["traceback"]) {
                std::string traceLine = line;
                // Remove ANSI escape codes
                traceLine = std::regex_replace(traceLine, std::regex("\x1b\\[[0-9;]*m"), "");
                html << escapeHTML(traceLine) << "\n";
            }
        }

        html << "</div>\n";
    }

    return html.str();
}

std::string JupyterParser::extractBase64Image(const std::string& base64Data,
                                                const std::string& mimeType,
                                                const std::string& notebookPath,
                                                int cellNumber, int outputNumber,
                                                std::vector<std::string>& extractedImages) {
    try {
        // Determine file extension
        std::string extension = ".png";
        if (mimeType == "image/jpeg") {
            extension = ".jpg";
        } else if (mimeType == "image/svg+xml") {
            extension = ".svg";
        }

        // Generate filename based on notebook name and cell/output numbers
        fs::path nbPath(notebookPath);
        std::string stem = nbPath.stem().string();
        std::string filename = stem + "_cell" + std::to_string(cellNumber) +
                                "_out" + std::to_string(outputNumber) + extension;

        // Create output directory if it doesn't exist
        fs::path imagesDir = "content/images/notebooks";
        if (!fs::exists(imagesDir)) {
            fs::create_directories(imagesDir);
        }

        // Full path for the image
        fs::path imagePath = imagesDir / filename;

        // Decode base64 and save image
        std::vector<unsigned char> imageData;
        std::string decoded = base64Decode(base64Data);
        imageData.assign(decoded.begin(), decoded.end());

        saveImage(imageData, imagePath.string());

        // Add to extracted images list
        extractedImages.push_back(imagePath.string());

        // Calculate correct relative path based on notebook location
        // Count depth by looking for how many directories deep the notebook is in content/blog/
        std::string pathStr = nbPath.string();
        int depth = 1;  // Default for content/blog/notebook.ipynb

        // Count slashes after "content/blog/" or "blog/"
        size_t blogPos = pathStr.find("content/blog/");
        if (blogPos == std::string::npos) {
            blogPos = pathStr.find("blog/");
        }

        if (blogPos != std::string::npos) {
            std::string afterBlog = pathStr.substr(blogPos + (pathStr.find("content/blog/") != std::string::npos ? 13 : 5));
            depth = 1;  // Start at 1 for files directly in blog/
            for (char c : afterBlog) {
                if (c == '/' || c == '\\') depth++;
            }
        }

        // Build relative path (e.g., "../images/" or "../../images/" or "../../../images/")
        std::string relativePath;
        for (int i = 0; i < depth; i++) {
            relativePath += "../";
        }
        relativePath += "images/notebooks/";

        // Return HTML img tag with relative path
        return "<img src=\"" + relativePath + filename + "\" alt=\"Output " +
               std::to_string(outputNumber) + "\" />";

    } catch (const std::exception& e) {
        return "<div class=\"error\">Error extracting image: " + std::string(e.what()) + "</div>";
    }
}

std::string JupyterParser::saveImage(const std::vector<unsigned char>& imageData,
                                      const std::string& outputPath) {
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + outputPath);
    }

    outFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    outFile.close();

    return outputPath;
}

std::string JupyterParser::escapeHTML(const std::string& str) {
    std::string result;
    result.reserve(str.size());

    for (char c : str) {
        switch (c) {
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '&':  result += "&amp;";  break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&#39;";  break;
            default:   result += c;        break;
        }
    }

    return result;
}

std::string JupyterParser::joinLines(const json& lines) {
    if (lines.is_string()) {
        return lines.get<std::string>();
    }

    if (!lines.is_array()) {
        return "";
    }

    std::stringstream result;
    for (const auto& line : lines) {
        if (line.is_string()) {
            result << line.get<std::string>();
        }
    }

    return result.str();
}

std::string JupyterParser::detectLanguage(const json& metadata) {
    if (metadata.contains("kernelspec") && metadata["kernelspec"].is_object()) {
        const json& kernelspec = metadata["kernelspec"];
        if (kernelspec.contains("language")) {
            return kernelspec["language"];
        }
    }

    return "python";  // Default
}

std::string JupyterParser::getPrismLanguageClass(const std::string& language) {
    if (language == "r" || language == "R") {
        return "language-r";
    } else if (language == "julia") {
        return "language-julia";
    } else if (language == "bash" || language == "shell") {
        return "language-bash";
    }

    return "language-python";  // Default
}

std::string JupyterParser::base64Decode(const std::string& encoded) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string decoded;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return decoded;
}
