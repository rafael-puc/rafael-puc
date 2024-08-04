# Start
$remove_appx = @(
        'SecHealthUI', 'PrintDialog', 'CrossDevice', 'PrintQueue',
        'DesktopSpotlight', 'StartMenuExperience', 'ParentalControls',
        'PeopleExperience', 'DevHome', 'NarratorQuickStart', 'CapturePicker', 'Remote'
    )
    $store = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Appx\AppxAllUserStore'
    $sids = Get-ChildItem -Path $store | Where-Object { $_.PSChildName -like 'S-1-5-*' }
    foreach ($choice in $remove_appx) {
        foreach ($sid in $sids.PSChildName) {
            $sidPath = \"$store\$sid\"
            if (Test-Path $sidPath) {
                $packageKeys = Get-ChildItem -Path $sidPath
                foreach ($packageKey in $packageKeys) {
                    $packageKeyPath = $packageKey.PSPath
                    $packageName = $packageKey.PSChildName
                    if ($packageName -like \"*$choice*\") {
                        $path = (Get-Item -LiteralPath $packageKeyPath).GetValue('Path', $null)
                        if ($path) {
                            $directory = Split-Path -Path $path -Parent
                            Remove-Item -Path $directory -Recurse -Force -ErrorAction SilentlyContinue
                        }
                        Remove-Item -Path $packageKeyPath -Recurse -Force -ErrorAction SilentlyContinue
                    }
                }
            }
        }
    }
# Remove Windows Defender Registry Key
    $defenderKey = 'HKLM:\SOFTWARE\Policies\Microsoft\Windows Defender'
    if (Test-Path $defenderKey) {
        Remove-Item -Path $defenderKey -Recurse -Force -ErrorAction SilentlyContinue
    }
# Mitigate WMI Autologger
    $wmiAutologgerPath = 'HKLM:\SYSTEM\CurrentControlSet\Control\WMI\Autologger\EventLog-System'
    if (Test-Path $wmiAutologgerPath) {
        Set-ItemProperty -Path $wmiAutologgerPath -Name 'Enabled' -Value 0 -Type DWord -ErrorAction SilentlyContinue
        Set-ItemProperty -Path $wmiAutologgerPath -Name 'EnableProperty' -Value 0 -Type DWord -ErrorAction SilentlyContinue
    }
# Remove Microsoft Spware and Bloatware
    $windowsRuntimePath = 'HKLM:\SOFTWARE\Microsoft\WindowsRuntime\ActivatableClassId'
    $subkeysToRemove = @('Speech', 'People', 'Shared', 'Touch', 'Bluetooth', 'Phone', 'Wifi', 'Cortana', 'Printer', 'Diagnostic', 'Feedback', 'SecurityCenter', 'Maps', 'RemoteDesktop', 'MachineLearning', 'Hydrogen')
    if (Test-Path $windowsRuntimePath) {
        $subkeys = Get-ChildItem -Path $windowsRuntimePath
        foreach ($subkey in $subkeys) {
            foreach ($removeName in $subkeysToRemove) {
                if ($subkey.PSChildName -like "*$removeName*") {
                    Remove-Item -Path $subkey.PSPath -Recurse -Force -ErrorAction SilentlyContinue
                }
            }
        }
    }
}

