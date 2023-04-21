for %%I in (.) do SET dirName=%%~nxI
set "formattedDate=%date%"
set "formattedDate=%formattedDate:/=_%"
set "formattedDate=%formattedDate:~4%" 
git archive --output=../gitBackups/%dirName%_%formattedDate%.zip HEAD
