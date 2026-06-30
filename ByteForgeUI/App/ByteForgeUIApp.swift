import SwiftUI

@main
struct ByteForgeUIApp: App {
    var body: some Scene {
        WindowGroup {
            ResearchDashboardView()
        }
        .windowResizability(.contentSize)
    }
}
