$chooseExampleType=$args[0]
$electronVersion=$args[1]
$example_sdk_mode=$args[2]
$outterZipName="electronDemo.zip"

pushd example

function ChooseArch($type)
{
  # remove node_modules
  Remove-Item -Path node_modules -Recurse -Force -ErrorAction Ignore;
  if($type -eq 1){
    write-host("ChooseArch x32")
    Copy-Item -Path ../ci/.npmrc_x32 -Destination ./.npmrc -Force
  } elseif($type -eq 2){
    write-host("ChooseArch x64")
    Copy-Item -Path ../ci/.npmrc_x64 -Destination ./.npmrc -Force
  }else {
    write-host("not set arch type")
  }
}

if($electronVersion -eq "switchEnv"){
    write-host("switchEnv")
    ChooseArch -type $chooseExampleType
    yarn
    popd
    return
}

function DistByArch($type)
{

  if($type -eq 1){
    write-host("distByArch x32")
    yarn dist:win32
  } elseif($type -eq 2){
    write-host("distByArch x64")
    yarn dist:win64
  }else {
    write-host("not set arch type")
  }
}

function Package($archNum,$electronVersion,$example_sdk_mode){
  # remove zip
  Remove-Item -Path ../$outterZipName -Recurse -Force -ErrorAction Ignore;
  
  # remove dist
  Remove-Item -Path dist -Recurse -Force -ErrorAction Ignore;

  # choose arch
  ChooseArch -type $archNum
  
  If([String]::IsNullOrEmpty($electronVersion))
  {
    Write-Host "安装example 依赖"
    yarn
  }
  Else
  {
    
    Write-Host "选择了 electron_version:$electronVersion"
    yarn add electron@$electronVersion
  }
  if($example_sdk_mode -eq 1){
    # copy native sdk
    Copy-Item -Path ../Electron-*/* -Destination src/node_modules/electron-agora-rtc-ng/ -Recurse -Force
  }
  
  # dist start
  DistByArch -type $archNum
  # move zip
  Copy-Item -Path dist/ElectronReact-*.zip -Destination ../$outterZipName -Recurse -Force
}

write-host("Package win:1")
Package -archNum $chooseExampleType -electronVersion $electronVersion -example_sdk_mode $example_sdk_mode
popd;

# switch -Regex ($chooseExampleType)
# {
#     1 {
#       write-host("Package win:1")
#       Package -archNum $args[1]
#       Break
#     }
#     2 {
#       write-host("Package win:2")
#       Package -archNum $args[1]
#       Break
#     }
#     3 {
#       write-host("Package win:3")
#       Package -archNum $args[1]
#       Break
#     }
#     4 {"It is four."; Break}
# }


echo "结束"
