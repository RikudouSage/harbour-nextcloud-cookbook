import QtQuick 2.0
import Sailfish.Silica 1.0

import "pages"
import "cover"
import "components"

ApplicationWindow {
    id: app

    property bool reportErrorsPageOpen: false

    function openReportErrorsPage() {
        if (reportErrorsPageOpen) {
            return;
        }

        reportErrorsPageOpen = true;
        var page = pageStack.push(Qt.resolvedUrl("pages/ReportErrorsPage.qml"));
        page.statusChanged.connect(function() {
            if (page.status === PageStatus.Inactive) {
                reportErrorsPageOpen = false;
                shakeDetector.reset();
            }
        });
    }

    initialPage: Component { CheckPage {} }
    cover: CoverPage {}
    allowedOrientations: defaultAllowedOrientations

    ShakeDetector {
        id: shakeDetector

        onShakeDetected: {
            app.openReportErrorsPage();
        }
    }
}
