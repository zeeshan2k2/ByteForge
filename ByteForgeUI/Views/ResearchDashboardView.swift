import SwiftUI

struct ResearchDashboardView: View {
    @StateObject private var viewModel = ResearchViewModel()

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                header
                sourceSection
                controlsSection
                if let result = viewModel.result {
                    resultsSection(result)
                    distributionSection(result.distribution)
                }
            }
            .padding(24)
        }
        .frame(minWidth: 980, minHeight: 760)
    }

    private var header: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("ByteForgeUI")
                .font(.system(size: 30, weight: .bold, design: .rounded))
            Text("Research dashboard for running the current ByteForge compression experiments against a sample or a custom file.")
                .foregroundStyle(.secondary)
            tokenWidthLegend
        }
    }

    private var tokenWidthLegend: some View {
        Grid(horizontalSpacing: 12, verticalSpacing: 12) {
            GridRow {
                MetricCard(title: "nib", value: "2 bits")
                MetricCard(title: "nibble", value: "4 bits")
                MetricCard(title: "sixbit", value: "6 bits")
                MetricCard(title: "byte", value: "8 bits")
            }
        }
        .padding(.top, 4)
    }

    private var sourceSection: some View {
        GroupBox("Input") {
            VStack(alignment: .leading, spacing: 14) {
                Picker("Source", selection: $viewModel.inputMode) {
                    ForEach(ResearchViewModel.InputMode.allCases) { mode in
                        Text(mode.rawValue).tag(mode)
                    }
                }
                .pickerStyle(.segmented)

                if viewModel.inputMode == .sample {
                    LabeledContent("Sample Path", value: viewModel.defaultSamplePath)
                        .textSelection(.enabled)
                } else {
                    HStack(spacing: 12) {
                        TextField("Custom file path", text: $viewModel.customPath)
                            .textFieldStyle(.roundedBorder)
                        Button("Browse") {
                            viewModel.browseForFile()
                        }
                    }
                }

                HStack(spacing: 12) {
                    Text("Bytes To Read")
                        .frame(width: 100, alignment: .leading)
                    TextField("0 for full file", text: $viewModel.bytesToReadText)
                        .textFieldStyle(.roundedBorder)
                        .frame(width: 180)
                    Text("Use `0` to read the full file.")
                        .foregroundStyle(.secondary)
                }
            }
            .padding(.top, 6)
        }
    }

    private var controlsSection: some View {
        GroupBox("Experiment") {
            VStack(alignment: .leading, spacing: 14) {
                Picker("Method", selection: $viewModel.selectedMethod) {
                    ForEach(ResearchViewModel.Method.allCases) { method in
                        Text(method.rawValue).tag(method)
                    }
                }
                .pickerStyle(.menu)
                .frame(width: 280)

                HStack(spacing: 14) {
                    Button(viewModel.isRunning ? "Running..." : "Run Analysis") {
                        viewModel.runAnalysis()
                    }
                    .buttonStyle(.borderedProminent)
                    .disabled(viewModel.isRunning)

                    Text(viewModel.statusMessage)
                        .foregroundStyle(.secondary)
                }
            }
            .padding(.top, 6)
        }
    }

    private func resultsSection(_ result: ByteForgeAnalysisResult) -> some View {
        GroupBox("Results") {
            VStack(alignment: .leading, spacing: 14) {
                Grid(horizontalSpacing: 14, verticalSpacing: 14) {
                    GridRow {
                        MetricCard(title: "Original Size", value: byteString(result.originalSize))
                        MetricCard(title: "Compressed Size", value: byteString(result.compressedSize))
                        MetricCard(title: "Saved", value: percentString(result.savedPercent))
                    }
                    GridRow {
                        MetricCard(title: "Compression Time", value: elapsedString(result.compressionMilliseconds))
                        MetricCard(title: "Decompression Time", value: elapsedString(result.decompressionMilliseconds))
                        MetricCard(title: "Total Time", value: elapsedString(result.totalMilliseconds))
                    }
                    GridRow {
                        MetricCard(title: "Chunk Count", value: "\(result.chunkCount)")
                        MetricCard(title: "Nibble Chunks", value: "\(result.nibbleChunkCount)")
                        MetricCard(title: "Raw Chunks", value: "\(result.rawChunkCount)")
                    }
                }

                Text(result.rebuildMatches ? "Rebuild matched the source bytes." : "Rebuild did not match the source bytes.")
                    .foregroundStyle(result.rebuildMatches ? .green : .red)
            }
            .padding(.top, 6)
        }
    }

    private func distributionSection(_ distribution: ByteForgeDistributionMetrics) -> some View {
        GroupBox("Byte Distribution") {
            VStack(alignment: .leading, spacing: 14) {
                Grid(horizontalSpacing: 14, verticalSpacing: 14) {
                    GridRow {
                        MetricCard(title: "Total Bytes", value: "\(distribution.totalBytes)")
                        MetricCard(title: "Distinct Values", value: "\(distribution.distinctByteCount)")
                        MetricCard(title: "Top 4 Coverage", value: percentString(distribution.top4Coverage))
                    }
                    GridRow {
                        MetricCard(title: "0 to < 4", value: percentString(distribution.zeroTo3Percent))
                        MetricCard(title: "4 to < 16", value: percentString(distribution.fourTo15Percent))
                        MetricCard(title: "16 to < 64", value: percentString(distribution.sixteenTo63Percent))
                    }
                    GridRow {
                        MetricCard(title: "64 to < 128", value: percentString(distribution.sixtyFourTo127Percent))
                        MetricCard(title: "128 to <= 255", value: percentString(distribution.oneTwentyEightTo255Percent))
                        MetricCard(title: "Top 64 Coverage", value: percentString(distribution.top64Coverage))
                    }
                }

                Text("This is the first analysis layer for deciding whether smaller custom token widths are even worth exploring.")
                    .foregroundStyle(.secondary)
            }
            .padding(.top, 6)
        }
    }

    private func byteString(_ value: UInt) -> String {
        ByteCountFormatter.string(fromByteCount: Int64(value), countStyle: .file)
    }

    private func percentString(_ value: Double) -> String {
        String(format: "%.2f%%", value)
    }

    private func elapsedString(_ milliseconds: Double) -> String {
        if milliseconds < 1000 {
            return String(format: "%.3f ms", milliseconds)
        }

        let seconds = milliseconds / 1000
        if seconds < 60 {
            return String(format: "%.3f s", seconds)
        }

        let minutes = seconds / 60
        if minutes < 60 {
            return String(format: "%.3f min", minutes)
        }

        let hours = minutes / 60
        return String(format: "%.3f hr", hours)
    }
}
