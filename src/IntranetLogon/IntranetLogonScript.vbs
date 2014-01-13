On Error Resume Next

If WScript.Arguments.Count = 2 Then
dim UserName
UserName = WScript.Arguments.Item(0)
dim UserPass
UserPass = WScript.Arguments.Item(1)

Set IE = WScript.CreateObject("InternetExplorer.Application")   
 IE.Visible = 1   
 IE.navigate "http://example.com"   
 Do While (IE.Busy)   
     WScript.Sleep 1000   
 Loop  

With IE.Document  
 .getElementById("edit-name").value = UserName   
 .getElementById("edit-pass").value = UserPass   
 .Forms(0).Submit   
End With
 
'*call msgbox("Logged user: '" + UserName + "', '" + UserPass + "'")

Else

call msgbox("Please provide command line arguments", vbOKOnly+vbCritical , "Error")

End If

On Error GoTo 0
