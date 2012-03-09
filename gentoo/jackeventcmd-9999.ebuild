# media-sound/jackeventcmd/jackeventcmd-9999.ebuild
EAPI=3

inherit git-2

DESCRIPTION="Run custom commands when headphones are (un)plugged"
HOMEPAGE="https://github.com/gentoo-root/jackeventcmd"
EGIT_REPO_URI="git://github.com/gentoo-root/jackeventcmd.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

RDEPEND="sys-apps/dbus
	media-sound/jacklistener"
DEPEND="${RDEPEND}"

src_install() {
	emake DESTDIR="${D}" install || die
	dodoc AUTHORS NEWS README* ChangeLog
}
