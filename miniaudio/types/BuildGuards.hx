package miniaudio.types;

#if hlopenal
#error "hlminiaudio and hlopenal is conflicting; remove one of them from the build."
#end
#if hlopus
#error "`hlminiaudio` provides native Opus support; remove `-lib hlopus` from the build."
#end
