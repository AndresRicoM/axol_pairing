Write-Output "Starting Serial Monitor... Logs will be saved to quality_logs.txt"

# Ejecutar pio device monitor y agregar marcas de tiempo
pio device monitor | ForEach-Object {
    $timestamp = Get-Date -Format "[HH:mm:ss]"
    "$timestamp $_"
} | Tee-Object -FilePath quality_logs.txt -Append