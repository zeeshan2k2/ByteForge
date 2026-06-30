import AppKit
import Combine
import Foundation

@MainActor
final class ResearchViewModel: ObservableObject {
    enum InputMode: String, CaseIterable, Identifiable {
        case sample = "Default 5000-byte Sample"
        case custom = "Custom File"

        var id: String { rawValue }
    }

    enum Method: String, CaseIterable, Identifiable {
        case repeatedByte = "Repeated-Byte Compression"
        case adaptiveNibble = "Adaptive Nibble Compression"

        var id: String { rawValue }

        var bridgeValue: ByteForgeCompressionMethod {
            switch self {
                case .repeatedByte:
                    return .repeatedByte
                case .adaptiveNibble:
                    return .adaptiveNibble
            }
        }
    }

    @Published var inputMode: InputMode = .sample
    @Published var customPath: String = ""
    @Published var bytesToReadText: String = "0"
    @Published var selectedMethod: Method = .adaptiveNibble
    @Published var isRunning = false
    @Published var statusMessage = "Pick a source and run an experiment."
    @Published var result: ByteForgeAnalysisResult?

    let defaultSamplePath = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/samples/synapse-qwen1.5b-first-5000.gguf"

    private let service = ByteForgeAnalysisService()

    var resolvedInputPath: String {
        inputMode == .sample ? defaultSamplePath : customPath
    }

    func browseForFile() {
        let panel = NSOpenPanel()
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false

        if panel.runModal() == .OK {
            customPath = panel.url?.path ?? ""
        }
    }

    func runAnalysis() {
        let inputPath = resolvedInputPath
        guard !inputPath.isEmpty else {
            statusMessage = "Choose a file path first."
            return
        }

        let bytesToRead = UInt(bytesToReadText) ?? 0
        isRunning = true
        statusMessage = "Running \(selectedMethod.rawValue)..."
        result = nil

        Task {
            let bridgeResult = await Task.detached(priority: .userInitiated) { [service, selectedMethod] in
                service.run(selectedMethod.bridgeValue, inputPath: inputPath, bytesToRead: bytesToRead)
            }.value

            isRunning = false
            result = bridgeResult
            statusMessage = bridgeResult.success
                ? "Finished. Rebuild \(bridgeResult.rebuildMatches ? "matched" : "did not match")."
                : bridgeResult.errorMessage
        }
    }
}
