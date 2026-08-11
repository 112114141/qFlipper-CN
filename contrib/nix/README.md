# qFlipper 的 Nix 打包


## 构建

__请检查您是否至少安装了 Nix 2.6。__

您可以通过一条命令使用 Nix 构建 qFlipper。

```
nix build --experimental-features "nix-command flakes" 'github:flipperdevices/qFlipper?dir=contrib/nix&submodules=1'
```

为简便起见，在后续示例中我将省略 `--experimental-features "nix-command flakes"`。[启用 flakes](#Enabling%20flakes) 即可省略这些选项。

如果您希望使用不同版本的 qFlipper，只需在仓库路径后指定提交哈希或修订版本号即可。


```
nix build 'github:flipperdevices/qFlipper/COMMIT_HASH_OR_REVISION?dir=contrib/nix&submodules=1'
```

_请注意，对于不包含 Nix Flake 文件的修订版本，此方法无效_

构建的二进制文件将位于 `./result/bin` 文件夹中。

## 开发

运行 `nix develop 'contrib/nix?submodules=1'`
这会让您进入一个包含所有必需构建依赖的 shell。
之后，您可以照常进行：
```
mkdir build
cd build
qmake ..
make -j30
```

您可能还需要对构建的二进制文件运行 `makeQtWrapper ./binary ./wrapper` 来生成包装器 shell 脚本，
该脚本会加载所有必需的 QT 环境变量。


### 启用 flakes
如果您希望以后再也不必输入 `--experimental-features "nix-command flakes"`：
```
mkdir -p ~/.config/nix
echo "experimental-features = nix-command flakes" >> ~/.config/nix/nix.conf
```
