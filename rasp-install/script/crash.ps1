param (
    [string]
    $crashFile,
    [parameter(mandatory = $true)]
    [string]
    $url,
    [parameter(mandatory = $true)]
    [string]
    $language
)

function FileReadable {
    param( [string]$FullPathName )
    try {
        [System.IO.File]::OpenRead($FullPathName).Close()
        $Readable = $true
    }
    catch {
        $Readable = $false        
    }
    return $Readable
}

$hostname = [System.Net.Dns]::GetHostName();

Add-Type -AssemblyName System.Net.Http
$multipartContent = New-Object System.Net.Http.MultipartFormDataContent

$stringHeader = New-Object System.Net.Http.Headers.ContentDispositionHeaderValue "form-data"
$stringHeader.Name = "language"
$StringContent = New-Object System.Net.Http.StringContent $language
$StringContent.Headers.ContentDisposition = $stringHeader
$multipartContent.Add($stringContent)

$stringHeader = New-Object System.Net.Http.Headers.ContentDispositionHeaderValue "form-data"
$stringHeader.Name = "job"
$StringContent = New-Object System.Net.Http.StringContent "crash"
$StringContent.Headers.ContentDisposition = $stringHeader
$multipartContent.Add($stringContent)

$stringHeader = New-Object System.Net.Http.Headers.ContentDispositionHeaderValue "form-data"
$stringHeader.Name = "hostname"
$StringContent = New-Object System.Net.Http.StringContent $hostname
$StringContent.Headers.ContentDisposition = $stringHeader
$multipartContent.Add($stringContent)

if (FileReadable($crashFile)) {
    $FileStream = New-Object System.IO.FileStream @($crashFile, [System.IO.FileMode]::Open)
    $fileHeader = New-Object System.Net.Http.Headers.ContentDispositionHeaderValue "form-data"
    $fileHeader.Name = "crash_log"
    $fileHeader.FileName = Split-Path -leaf $crashFile
    $fileContent = New-Object System.Net.Http.StreamContent $FileStream
    $fileContent.Headers.ContentDisposition = $fileHeader
    $fileContent.Headers.ContentType = [System.Net.Http.Headers.MediaTypeHeaderValue]::Parse("text/plain")
    $multipartContent.Add($fileContent)
}

$HttpClient = New-Object System.Net.Http.HttpClient
$HttpClient.Timeout = New-Object System.TimeSpan(0, 0, 5);
$response = $httpClient.PostAsync($url, $multipartContent).Result
$response.EnsureSuccessStatusCode()
$httpClient.Dispose()
